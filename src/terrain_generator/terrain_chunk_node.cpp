#include "terrain_chunk_node.h"
#include "chunkinator/image_sampling.h"
#include "godot_cpp/classes/collision_shape3d.hpp"
#include "godot_cpp/classes/concave_polygon_shape3d.hpp"
#include "godot_cpp/classes/static_body3d.hpp"
#include "godot_cpp/classes/worker_thread_pool.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/templates/hash_set.hpp"

void TerrainChunkNode::_update_quadtree_task() {
    quadtree->clear();
    const Vector2 local_camera_pos = current_quadtree_task.camera_position - world_rect.position;
    quadtree->insert_camera(local_camera_pos);
    LocalVector<QuadTree::LeafInformation> leaves;
    HashSet<Rect2i> leaves_set;
    leaves_set.reserve(leaves.size());

    quadtree->get_leafs(leaves);

    for (const QuadTree::LeafInformation &leaf : leaves) {
        leaves_set.insert(leaf.rect);
    }

    // Mark dead chunks for unloading
    current_quadtree_task.chunks_to_unload.clear();
    for (KeyValue<Rect2i, TerrainChunkInformation> &it : chunks) {
        if (!leaves_set.has(it.key)) {
            it.value.status = NEEDS_UNLOADNG;
            current_quadtree_task.chunks_to_unload.push_back(it.key);
        }
    }
    for (const QuadTree::LeafInformation &leaf : leaves) {
        auto existing_chunk = chunks.find(leaf.rect);
        if (existing_chunk != chunks.end()) {
            if (leaf.lod_mask != existing_chunk->value.lod_mask) {
                existing_chunk->value.lod_mask = leaf.lod_mask;
                existing_chunk->value.lod_mask_needs_updating = true;
            }
            continue;
        }
        // Add new chunks
        chunks.insert(leaf.rect, {
            .lod_mask = leaf.lod_mask,
            .lod_depth = leaf.depth
        });
    }
}

void TerrainChunkNode::_generate_collision_meshes_task(uint32_t p_idx) {
    const Rect2i current_chunk = current_mesh_generation_task.chunks_that_need_collision_mesh[p_idx];
    const int collision_mesh_resolution = 19;

    const int vertex_count = collision_mesh_resolution*collision_mesh_resolution;

    PackedVector3Array vertices;
    vertices.resize(vertex_count);

    // Get the heightmap image
    const int heightmap_size = heightmap->get_size().x;
    const Rect2 chunk_local_rect = current_chunk;

    const Vector2 local_rect_pos_normalized = Vector2(chunk_local_rect.position) / (double)superchunk_size; // Normalize to [0,1]
    const Vector2 local_rect_size_normalized = Vector2(chunk_local_rect.size) / (double)superchunk_size;
    for (int x = 0; x < collision_mesh_resolution; x++) {
        const double x_v = (x / (double)(collision_mesh_resolution - 1));
        for (int y = 0; y < collision_mesh_resolution; y++) {
            const double y_v = (y / (double)(collision_mesh_resolution - 1));
            const int idx = y * collision_mesh_resolution + x;

            // Map collision mesh coordinates to heightmap coordinates
            Vector2 sample_pos_local = local_rect_pos_normalized + Vector2(x_v, y_v) * local_rect_size_normalized;

            // Convert to heightmap pixel coordinates
            double u = sample_pos_local.x;
            double v = sample_pos_local.y;

            if (world_rect.position == Vector2i() && current_chunk.position == Vector2i()) {
                print_line(bilinearly_sample_image_single_channel(heightmap, 3, Vector2(u, v)));
            }

            vertices[idx] = Vector3(x_v * chunk_local_rect.size.x, bilinearly_sample_image_single_channel(heightmap, 3, Vector2(u, v)), y_v * chunk_local_rect.size.y);
        }
    }

    PackedVector3Array out_vertices;
    out_vertices.resize(vertex_count*6);
    int xid = 0;
    for (int x = 0; x < collision_mesh_resolution-1; x++) {
        const int xi = x * 6 * collision_mesh_resolution;
        for (int y = 0; y < collision_mesh_resolution-1; y++) {
            const int yi = y * 6;
            const int top_left_idx = y * collision_mesh_resolution + x;
            const int top_right_idx = y * collision_mesh_resolution + x + 1;
            const int bottom_right_idx = (y + 1) * collision_mesh_resolution + x + 1;
            const int bottom_left_idx = (y + 1) * collision_mesh_resolution + x;
            
            out_vertices[xid++] = vertices[top_left_idx];
            out_vertices[xid++] = vertices[top_right_idx];
            out_vertices[xid++] = vertices[bottom_left_idx];
    
            out_vertices[xid++] = vertices[top_right_idx];
            out_vertices[xid++] = vertices[bottom_right_idx];
            out_vertices[xid++] = vertices[bottom_left_idx];
        }
    }

    Ref<ConcavePolygonShape3D> shape;
    shape.instantiate();
    shape->set_faces(out_vertices);
    chunks[current_chunk].shape = shape;
}

void TerrainChunkNode::_check_quadtree_generation() {
    WorkerThreadPool *wtp = WorkerThreadPool::get_singleton();
    if (!wtp->is_task_completed(current_quadtree_task.quadtree_generation_task_id)) {
        return;
    }
    wtp->wait_for_task_completion(current_quadtree_task.quadtree_generation_task_id);

    // Prepare for creating collision meshes
    current_mesh_generation_task.chunks_that_need_collision_mesh.clear();
    for (KeyValue<Rect2i, TerrainChunkInformation> &it : chunks) {
        if (it.value.status == ChunkStatus::NEEDS_INSTANCING) {
            // We can instance this, but we also need the collision mesh
            current_mesh_generation_task.chunks_that_need_collision_mesh.push_back(it.key);
        }
    }

    // No collision meshes to generate, we are done.
    if (current_mesh_generation_task.chunks_that_need_collision_mesh.is_empty()) {
        status = UpdateStatus::IDLING;
        _unload_queued_chunks();
        _update_chunk_lod_masks();
        return;
    }
    
    status = UpdateStatus::GENERATING_COLLISION_MESH;

    // We need to generate collision meshes
    current_mesh_generation_task.task_id = wtp->add_group_task(callable_mp(this, &TerrainChunkNode::_generate_collision_meshes_task), current_mesh_generation_task.chunks_that_need_collision_mesh.size());
}

void TerrainChunkNode::_check_collision_mesh_generation() {
    WorkerThreadPool *wtp = WorkerThreadPool::get_singleton();
    DEV_ASSERT(current_mesh_generation_task.task_id != -1);
    if (!wtp->is_group_task_completed(current_mesh_generation_task.task_id)) {
        return;
    }
    wtp->wait_for_group_task_completion(current_mesh_generation_task.task_id);

    // Now we can actually instantiate stuff...
    for (KeyValue<Rect2i, TerrainChunkInformation> &it : chunks) {
        if (it.value.status != ChunkStatus::NEEDS_INSTANCING) {
            continue;
        }
        DEV_ASSERT(it.value.mesh_instance == nullptr);
        it.value.mesh_instance = memnew(MeshInstance3D);
        add_child(it.value.mesh_instance);
        it.value.mesh_instance->set_global_position( Vector3(it.key.position.x + world_rect.position.x, 0.0, it.key.position.y + world_rect.position.y));
        it.value.mesh_instance->set_mesh(plane_mesh);
        material->set_shader_parameter("heightmap", heightmap_texture);
        material->set_shader_parameter("road_sdf", road_sdf_texture);
        
        Vector2 origin = Vector2(it.key.position) / Vector2(world_rect.size);
        Vector2 end = Vector2(it.key.position + it.key.size) / Vector2(world_rect.size);
        
        it.value.mesh_instance->set_instance_shader_parameter("side_size", it.key.size.x);
        it.value.mesh_instance->set_instance_shader_parameter("lod_depth", it.value.lod_depth);
        AABB aabb;
        aabb.set_position(Vector3(0.0, -3000.0, 0.0));
        aabb.set_size(Vector3(it.key.size.x, 6000.0, it.key.size.y));
        
        it.value.mesh_instance->set_custom_aabb(aabb);
        
        it.value.status = ChunkStatus::DONE;

        it.value.collision_shape = memnew(CollisionShape3D);
        it.value.body = memnew(StaticBody3D);
        it.value.body->add_child(it.value.collision_shape);
        it.value.mesh_instance->add_child(it.value.body);
        it.value.collision_shape->set_shape(it.value.shape);
        //it.value.body->set_global_position(Vector3(world_rect.position.x +, 0.0, world_rect.position.y));
    }

    // ... and unload what we don't need
    _unload_queued_chunks();
    _update_chunk_lod_masks();

    status = UpdateStatus::IDLING;
}

void TerrainChunkNode::_unload_queued_chunks() {
    auto it = chunks.begin();

    for (Rect2i chunk : current_quadtree_task.chunks_to_unload) {
        chunks[chunk].mesh_instance->queue_free();
        chunks.erase(chunk);
    }
}

void TerrainChunkNode::_update_chunk_lod_masks() {
    for (KeyValue<Rect2i, TerrainChunkInformation> &chunk : chunks) {
        if (!chunk.value.lod_mask_needs_updating) {
            continue;
        }
        chunk.value.lod_mask_needs_updating = false;
        chunk.value.mesh_instance->set_instance_shader_parameter("lod_mask", chunk.value.lod_mask);
    }
}

void TerrainChunkNode::initialize(const TerrainChunkNodeInitializationProperties &p_init_properties) {

    DEV_ASSERT(p_init_properties.height_texture.is_valid());
    DEV_ASSERT(p_init_properties.heightmap.is_valid());
    DEV_ASSERT(p_init_properties.road_sdf_texture.is_valid());
    DEV_ASSERT(p_init_properties.material.is_valid());
    DEV_ASSERT(p_init_properties.plane_mesh.is_valid());

    quadtree = QuadTree::create(p_init_properties.superchunk_size, 5);    
    heightmap = p_init_properties.heightmap;
    heightmap_texture = p_init_properties.height_texture;
    superchunk_size = p_init_properties.superchunk_size;
    world_rect = Rect2(p_init_properties.chunk_idx * superchunk_size, Vector2(superchunk_size, superchunk_size));
    plane_mesh = p_init_properties.plane_mesh->duplicate();
    road_sdf_texture = p_init_properties.road_sdf_texture;
    material = p_init_properties.material->duplicate();
    material->set_shader_parameter("superchunk_origin", Vector2(world_rect.position));
    material->set_shader_parameter("superchunk_size", Vector2(world_rect.size));
    material->set_shader_parameter("lod_max_depth", 4);
    plane_mesh->surface_set_material(0, material);
    set_global_position(Vector3(world_rect.position.x, 0.0, world_rect.position.y));
}

void TerrainChunkNode::update(Vector2i p_camera_position) {
    if (status == UpdateStatus::GENERATING_QUADTREE) {
        _check_quadtree_generation();
    }

    if (status == UpdateStatus::GENERATING_COLLISION_MESH) {
        _check_collision_mesh_generation();
    }

    if (status != UpdateStatus::IDLING) {
        return;
    }

    if (current_quadtree_task.has_camera_position && p_camera_position == current_quadtree_task.camera_position) {
        return;
    }

    WorkerThreadPool *wtp = WorkerThreadPool::get_singleton();
    current_quadtree_task.quadtree_generation_task_id = wtp->add_task(callable_mp(this, &TerrainChunkNode::_update_quadtree_task));
    current_quadtree_task.camera_position = p_camera_position;
    current_quadtree_task.has_camera_position = true;

    if (current_quadtree_task.quadtree_generation_task_id == -1) {
        return;
    }

    status = UpdateStatus::GENERATING_QUADTREE;
}

TerrainChunkNode::TerrainChunkNode()
{
}

