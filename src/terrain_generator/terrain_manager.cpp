#include "terrain_manager.h"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/templates/hash_set.hpp"
#include "terrain_generator/terrain_chunk_node.h"
#include "terrain_generator/terrain_heightmap_combine_layer.h"
#include "terrain_generator/terrain_roads.h"

void TerrainManager::_on_generation_completed() {
    Ref<TerrainFinalCombineLayer> layer = get_terrain_layer();
    HashSet<Vector2i> new_chunks;
    // Add new superchunks
    for (int i = 0; i < layer->get_chunk_count(); i++) {
        Ref<TerrainFinalCombineChunk> chunk = layer->get_chunk(i);
        const Vector2i chunk_idx = chunk->get_chunk_index();
        new_chunks.insert(chunk->get_chunk_index());
        if (!superchunks.has(chunk_idx)) {
            // Add new superchunk to tree
            TerrainSuperchunk superchunk = {
                .node = memnew(TerrainChunkNode)
            };
            add_child(superchunk.node);
            Ref<ImageTexture> height_texture = ImageTexture::create_from_image(chunk->get_height_map());
            Ref<ImageTexture> road_sdf_texture = ImageTexture::create_from_image(chunk->get_road_sdf());
            Ref<ShaderMaterial> mat = ResourceLoader::get_singleton()->load("res://test_shader.tres");
            superchunk.node->initialize({
                .superchunk_size = chunk->get_chunk_bounds().size.x,
                .chunk_idx = chunk_idx,
                .heightmap = chunk->get_height_map(),
                .height_texture = height_texture,
                .road_sdf_texture = road_sdf_texture,
                .plane_mesh = plane_mesh,
                .material = mat
            });
            
            superchunks.insert(chunk_idx, superchunk);
        }
    }
    // Delete existing ones that don't matter anymore
    HashMap<Vector2i, TerrainSuperchunk>::Iterator it = superchunks.begin();
    while (it != superchunks.end()) {
        if (!new_chunks.has(it->key)) {
            const Vector2i chunk_idx = it->key;
            ++it;
            _unload_superchunk(chunk_idx);
            continue;
        }
        ++it;
    }
}

void TerrainManager::_bind_methods() {
    
}

void TerrainManager::set_chunkinator(Ref<Chunkinator> p_chunkinator) {
    chunkinator = p_chunkinator;
    chunkinator->connect("generation_completed", callable_mp(this, &TerrainManager::_on_generation_completed));
}

void TerrainManager::_unload_superchunk(const Vector2i &p_idx) {
    auto it = superchunks.find(p_idx);
    DEV_ASSERT(it != superchunks.end());
    it->value.node->queue_free();
    superchunks.erase(p_idx);
}

void TerrainManager::set_camera_position(Vector3 p_camera_position) {
    camera_position = p_camera_position;
}

Ref<TerrainFinalCombineLayer> TerrainManager::get_terrain_layer() const {
    return chunkinator->get_layer("Heightmap Combine Layer");
}

Ref<TerrainRoadConnectionLayer> TerrainManager::get_road_layer() const {
    return chunkinator->get_layer("Road Connections");
}

void TerrainManager::update() {
    for (KeyValue<Vector2i, TerrainSuperchunk> &kv : superchunks) {
        kv.value.node->update(Vector2i(camera_position.x, camera_position.z));
    }   
}

void TerrainManager::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_READY: {
            plane_mesh = LODMesh::generate_mesh(mesh_quality);
        } break;
        case NOTIFICATION_PROCESS:  {
        } break;
    }
}
