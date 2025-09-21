#include "worldman.h"

#include "godot_cpp/templates/hash_set.hpp"
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/rendering_server.hpp"
#include "godot_cpp/classes/rendering_device.hpp"
#include "godot_cpp/classes/static_body3d.hpp"
#include "godot_cpp/classes/time.hpp"
#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/concave_polygon_shape3d.hpp"
#include "godot_cpp/classes/collision_shape3d.hpp"
#include "godot_cpp/classes/window.hpp"
#include "godot_cpp/classes/resource_saver.hpp"
#include "debugger/worldman_debugger.h"

void WorldMan::_update_task(void *p_userdata, uint32_t p_element_idx) {
    WorldMan *me = (WorldMan*)p_userdata;
    WorldManSuperchunk &superchunk = me->superchunks[p_element_idx];

    const int superchunk_data_size = 256;

    PackedInt32Array indices;
    indices.resize(superchunk_data_size*superchunk_data_size);
    PackedVector2Array vertices;
    
    if (superchunk.state == WorldManSuperchunk::State::CREATING) {
        Ref<Image> img = Image::create_empty(superchunk_data_size, superchunk_data_size, false, Image::Format::FORMAT_RGBAF);

        // Generate road data, if needed
        

        for (int x = 0; x < superchunk_data_size; x++) {
            const double x_v = (x / (double)(superchunk_data_size-1));
            for  (int y = 0; y < superchunk_data_size; y++) {
                const double y_v = (y / (double)(superchunk_data_size-1));

                const double SAMPLE_NUDGE = 50.0;

                Vector2 sample_center = Vector2(x_v, y_v) * SUPERCHUNK_SIZE;
                const double hL = me->sample_superchunk_noise(superchunk, sample_center - Vector2(SAMPLE_NUDGE, 0.0));
                const double hR = me->sample_superchunk_noise(superchunk, sample_center + Vector2(SAMPLE_NUDGE, 0.0));
                const double hD = me->sample_superchunk_noise(superchunk, sample_center - Vector2(0.0, SAMPLE_NUDGE));
                const double hU = me->sample_superchunk_noise(superchunk, sample_center + Vector2(0.0, SAMPLE_NUDGE));

                // deduce terrain normal
                Vector3 normal;
                normal.x = hL - hR;
                normal.y = SAMPLE_NUDGE*2.0;
                normal.z = hD - hU;
                normal.normalize();

                img->set_pixel(x, y, Color(normal.x, normal.y, normal.z, me->sample_superchunk_noise(superchunk, sample_center)));
            }
        }

        superchunk.base_normal_image = img;
        superchunk.base_normal_texture = ImageTexture::create_from_image(img);

        //RenderingServer *rs = RenderingServer::get_singleton();
        //RID rd_texture = rs->texture_get_rd_texture(superchunk.base_normal_texture->get_rid());
        //rs->get_rendering_device()->set_resource_name(rd_texture, vformat("Superchunk %d, %d base texture", superchunk.position.x, superchunk.position.y));
        superchunk.state = WorldManSuperchunk::State::NEEDS_INSTANCING;

        superchunk.quadtree = QuadTree::create(WorldMan::SUPERCHUNK_SIZE, WorldMan::SUPERCHUNK_MAX_DEPTH);

        me->_create_superchunk_roads(superchunk);
    }


    // Transform camera to superchunk space
    Transform2D superchunk_space_transform;
    superchunk_space_transform.set_origin(superchunk.position * WorldMan::SUPERCHUNK_SIZE);
    
    const Vector2 local_camera_pos = superchunk_space_transform.affine_inverse().xform(me->current_task.camera_position);
    superchunk.quadtree->clear();
    superchunk.quadtree->insert_camera(local_camera_pos);

    // Now we grab the new leafs and figure out what needs deleting and what needs to be added to the tree
    // first, delete

    LocalVector<QuadTree::LeafInformation> leaves;
    superchunk.quadtree->get_leafs(leaves);

    HashSet<Rect2i> existing_visible_chunk_rects;
    {
        HashMap<Rect2i, const QuadTree::LeafInformation*> visible_chunk_rects;

        visible_chunk_rects.reserve(leaves.size());

        for (const QuadTree::LeafInformation &leaf : leaves) {
            visible_chunk_rects.insert(leaf.rect, &leaf);
        }

        for (int i = 0; i < superchunk.chunks.size(); i++) {
            WorldManChunk &chunk = superchunk.chunks[i];
            if (!visible_chunk_rects.has(chunk.rect)) {
                chunk.state = WorldManChunk::ChunkState::NEEDS_UNLOADING;
                //me->unload_chunk(superchunk, i);
                //i--;
                continue;
            }
            existing_visible_chunk_rects.insert(chunk.rect);
            // Update existing chunk
            chunk.lod_mask = visible_chunk_rects[chunk.rect]->lod_mask;
        }
    }

    // We know which chunks survived, now to add the new chunks & update existing ones
    for(QuadTree::LeafInformation &leaf : leaves) {
        if (existing_visible_chunk_rects.has(leaf.rect)) {
            continue;
        }
        WorldMan::WorldManChunk chunk = {
            .rect = leaf.rect,
            .depth = leaf.depth
        };

        chunk.collision_shape = me->generate_collision_mesh_for_rect(superchunk, chunk.rect);

        superchunk.chunks.push_back(chunk);
    }
}

Ref<ConcavePolygonShape3D> WorldMan::generate_collision_mesh_for_rect(const WorldMan::WorldManSuperchunk &p_superchunk, const Rect2i &p_local_rect) const {
    const int collision_mesh_resolution = 19;

    const int vertex_count = collision_mesh_resolution*collision_mesh_resolution;

    PackedVector3Array vertices;
    vertices.resize(vertex_count);

    // Get the heightmap image
    Ref<Image> heightmap = p_superchunk.base_normal_image;
    const int heightmap_size = heightmap->get_size().x;

    for (int x = 0; x < collision_mesh_resolution; x++) {
        const double x_v = (x / (double)(collision_mesh_resolution - 1));
        for (int y = 0; y < collision_mesh_resolution; y++) {
            const double y_v = (y / (double)(collision_mesh_resolution - 1));
            const int idx = y * collision_mesh_resolution + x;

            // Map collision mesh coordinates to heightmap coordinates
            Vector2 rect_pos = Vector2(p_local_rect.position) / (double)SUPERCHUNK_SIZE; // Normalize to [0,1]
            Vector2 rect_size = Vector2(p_local_rect.size) / (double)SUPERCHUNK_SIZE;
            Vector2 sample_pos_local = rect_pos + Vector2(x_v, y_v) * rect_size;

            // Convert to heightmap pixel coordinates
            double u = sample_pos_local.x;
            double v = sample_pos_local.y;

            vertices[idx] = Vector3(x_v * p_local_rect.size.x, bilinearly_sample_image_single_channel(heightmap, 3, Vector2(u, v)), y_v * p_local_rect.size.y);
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
    return shape;
}

double WorldMan::sample_superchunk_noise(const WorldMan::WorldManSuperchunk &p_superchunk, const Vector2 &p_local_position) const {
	const Vector2 pos = Vector2(p_superchunk.position * SUPERCHUNK_SIZE) + p_local_position;
    return noise->get_noise_2dv(pos * 0.01) * 500.0;
}

void WorldMan::_update_task_finished() {
    WorkerThreadPool *wp = WorkerThreadPool::get_singleton();

    if (current_task.created_superchunks > 0) {
        const uint64_t end = Time::get_singleton()->get_ticks_usec();
        print_line(vformat("Created %d superchunks in %d usec!", current_task.created_superchunks, end-current_task.task_start_time_usec));
    }

    wp->wait_for_group_task_completion(current_task.task_id);
    current_task.task_id = -1;
    update_state = UpdateState::IDLE;

    for(WorldManSuperchunk &superchunk : superchunks) {
        const Vector2 superchunk_origin = superchunk.position * SUPERCHUNK_SIZE;
        
        if (superchunk.state == WorldManSuperchunk::State::NEEDS_INSTANCING) {
            superchunk.state = WorldManSuperchunk::State::IDLE;
            superchunk.material->set_shader_parameter(StringName("test_image"), superchunk.base_normal_texture);
            superchunk.material->set_shader_parameter(StringName("superchunk_position"), superchunk.position);
            superchunk.material->set_shader_parameter(StringName("superchunk_size"), SUPERCHUNK_SIZE);
            RenderingServer * rs = RenderingServer::get_singleton();
            if (superchunk.road_info.has_value()) {
                RID rd_rid = rs->texture_get_rd_texture(superchunk.road_info->road_spline_texture->get_rid());
                RenderingDevice *rd = rs->get_rendering_device();
                rd->set_resource_name(rd_rid, vformat("Road texture %d %d", superchunk.position.x, superchunk.position.y));
            }
            superchunk.material->set_shader_parameter(StringName("road_texture"), superchunk.road_info.has_value() ? superchunk.road_info->road_spline_texture : Ref<Texture2D>(empty_road_texture));
            superchunk.material->set_shader_parameter(StringName("has_road"), superchunk.road_info.has_value());
        }

        for (int i = 0; i < superchunk.chunks.size(); i++) {
            WorldManChunk &chunk = superchunk.chunks[i];
            if (chunk.state == WorldManChunk::ChunkState::NEEDS_UNLOADING) {
                unload_chunk(superchunk, i);
                --i;
                continue;
            }
            if (chunk.state == WorldManChunk::ChunkState::NEEDS_MESH_INSTANCE_INSTANTIATION) {
                chunk.state = WorldManChunk::ChunkState::OK;
                chunk.mesh_instance = memnew(MeshInstance3D);
                chunk.mesh_instance->set_mesh(superchunk.mesh);
                add_child(chunk.mesh_instance);
                const Vector2 chunk_origin = superchunk_origin + chunk.rect.position;
                chunk.mesh_instance->set_global_position(Vector3(chunk_origin.x, 0.0, chunk_origin.y));
                AABB aabb;
                aabb.set_position(Vector3(0.0, -1500.0, 0.0));
                aabb.set_size(Vector3(chunk.rect.size.x, 3000.0, chunk.rect.size.y));
                chunk.mesh_instance->set_custom_aabb(aabb);

                if (chunk.collision_shape.is_valid()) {
                    chunk.collision_body = memnew(StaticBody3D);
                    CollisionShape3D *cs = memnew(CollisionShape3D);
                    cs->set_shape(chunk.collision_shape);
                    chunk.collision_body->add_child(cs);
                    chunk.mesh_instance->add_child(chunk.collision_body);
                }
            }
            chunk.mesh_instance->set_instance_shader_parameter(StringName("LOD_REDUCTION_MASK"), chunk.lod_mask);
            chunk.mesh_instance->set_instance_shader_parameter(StringName("SIDE_SIZE"), (double)(chunk.rect.size.x));
            chunk.mesh_instance->set_instance_shader_parameter(StringName("LOD_COLOR"), Color(0.0, 1.0, 0.0, 1.0).lerp(Color(1.0, 0.0, 0.0, 1.0), chunk.depth / (double)(SUPERCHUNK_MAX_DEPTH-1)));
            chunk.mesh_instance->set_instance_shader_parameter(StringName("CHUNK_POSITION"), Vector2(chunk.rect.position));
        }
    }
}

void WorldMan::_create_superchunk_roads(WorldManSuperchunk &p_superchunk) {
    if (p_superchunk.position.x != 0) {
        // only x=0 superchunks have roads... for now 
        return;
    }

    Ref<Image> road_img = Image::create(ROAD_SPLINE_COUNT_PER_SUPERCHUNK+2, 1, false, Image::Format::FORMAT_RGBAF);
    
    Ref<Curve2D> superchunk_road_curve;
    superchunk_road_curve.instantiate();
    superchunk_road_curve->set_point_count(ROAD_SPLINE_COUNT_PER_SUPERCHUNK+2);

    // We add one extra from the surrounding chunks, just in case
    for (int i = -1; i < ROAD_SPLINE_COUNT_PER_SUPERCHUNK+1; i++) {
        const double local_z_n = (i / (ROAD_SPLINE_COUNT_PER_SUPERCHUNK-1.0f));
        
        const int global_spline_sample_idx = (p_superchunk.position.y * (ROAD_SPLINE_COUNT_PER_SUPERCHUNK-1))+i;
        const float sample_point = (global_spline_sample_idx / (ROAD_SPLINE_COUNT_PER_SUPERCHUNK-1.0f));
        const double x_offset = noise->get_noise_1d(sample_point * 200.0f) * 600.0f;
        
        Vector2 road_node_pos = Vector2(0.5f * (SUPERCHUNK_SIZE) + x_offset, local_z_n * SUPERCHUNK_SIZE);
        float height = 0.0f;
        if (i >= 0 && i < ROAD_SPLINE_COUNT_PER_SUPERCHUNK) {
            height = bilinearly_sample_image_single_channel(p_superchunk.base_normal_image, 3, road_node_pos / SUPERCHUNK_SIZE);
        } else {
            const Vector2 pos = (p_superchunk.position * SUPERCHUNK_SIZE) + road_node_pos;
            height = noise->get_noise_2dv(pos * 0.01) * 500.0;
        }
        
        Vector3 local_road_position = Vector3(road_node_pos.x, height, road_node_pos.y);


        road_img->set_pixel(i+1, 0, Color(local_road_position.x, local_road_position.y, local_road_position.z));
        if (p_superchunk.position.y == 0) {
            print_line(vformat("remap 0 %d %d %.2f %f", i, global_spline_sample_idx, sample_point, x_offset));
        }
        if (p_superchunk.position.y == 1) {
            print_line(vformat("remap 1 %d %d %.2f %f", i, global_spline_sample_idx, sample_point, x_offset));
        }
    }

    if (p_superchunk.position.y == 0 || p_superchunk.position.y == -1) {
        ResourceSaver::get_singleton()->save(road_img, vformat("res://cs_%d_%d.tres", p_superchunk.position.x, p_superchunk.position.y));
    }
    
    p_superchunk.road_info = WorldManSuperchunkRoadInfo {
        .road_spline_texture = ImageTexture::create_from_image(road_img)
    };
}

void WorldMan::apply_road_height(const WorldManSuperchunk &p_superchunk, Vector2 p_local_position, float &p_height) {
    for (int i = 0; i < ROAD_SPLINE_COUNT_PER_SUPERCHUNK+1; i++) {
        const Vector3 pos_a = p_superchunk.road_info->road_spline_positions[i];
        const Vector3 pos_b = p_superchunk.road_info->road_spline_positions[i+1];
        const Vector2 pos_a_2d = Vector2(pos_a.x, pos_a.z);
        const Vector2 a_to_b =  - Vector2(pos_b.x, pos_b.z);
        const float dist = a_to_b.length();

        //(p_local_position - pos_a_2d).dot(a_to_b)
        //p_local_position.project();
    }
}

void WorldMan::_bind_methods() {
	ClassDB::bind_method(D_METHOD("update", "camera_position"), &WorldMan::update);
    ClassDB::bind_method(D_METHOD("get_visible_superchunks"), &WorldMan::get_visible_superchunks_bind);
    ClassDB::bind_static_method(StringName("WorldMan"), D_METHOD("create", "terrain_shader"), &WorldMan::create);
}

void WorldMan::unload_superchunk(Vector2i p_superchunk) {
    WorldManSuperchunk *sc = find_superchunk(p_superchunk);
    for (int i = 0; i < sc->chunks.size(); i++) {
        unload_chunk(*sc, i);
        i--;
    }
}

void WorldMan::create_superchunk(Vector2i p_superchunk) {
    ERR_FAIL_COND_MSG(find_superchunk(p_superchunk) != nullptr, "Tried to create a superchunk that already exists");
    WorldManSuperchunk superchunk = {};
    superchunk.position = p_superchunk;
    superchunk.material = terrain_shader->duplicate();
    superchunk.mesh = chunk_mesh->duplicate();
    superchunk.mesh->surface_set_material(0, superchunk.material);
    superchunks.push_back(superchunk);
}

void WorldMan::unload_chunk(WorldManSuperchunk &p_superchunk, int p_chunk_idx) {
    WorldManChunk &chunk = p_superchunk.chunks[p_chunk_idx];
    if (chunk.mesh_instance != nullptr) {
        chunk.mesh_instance->queue_free();
    }
    p_superchunk.chunks.remove_at_unordered(p_chunk_idx);
}

TypedArray<Vector2i> WorldMan::get_visible_superchunks_bind() const {
	TypedArray<Vector2i> out;
    out.resize(superchunks.size());

    for (int i = 0; i < superchunks.size(); i++) {
        out[i] = superchunks[i].position;
    }
    return out;
}

void WorldMan::update(Vector3 p_camera_position) {
    if (current_task.task_id != -1) {
        WorkerThreadPool *wp = WorkerThreadPool::get_singleton();
        _update_task_finished();
    }

    if (update_state != UpdateState::IDLE) {
        return;
    }
    HashSet<Vector2i> visible_superchunks;
    {
        // First we calculate the visible superchunks
        Vector2i current_superchunk = Vector2i(p_camera_position.x / SUPERCHUNK_SIZE, p_camera_position.z / SUPERCHUNK_SIZE);
        for (int x = current_superchunk.x-SUPERCHUNK_RADIUS; x < current_superchunk.x + SUPERCHUNK_RADIUS+1; x++) {
            for (int y = current_superchunk.y-SUPERCHUNK_RADIUS; y < current_superchunk.y + SUPERCHUNK_RADIUS+1; y++) {
                const Vector2i superchunk_pos = Vector2i(x, y);
                const double distance = Vector2(superchunk_pos).distance_to(current_superchunk);
                if (distance <= SUPERCHUNK_RADIUS) {
                    visible_superchunks.insert(superchunk_pos);
                }
            }
        }
    }

    {
        // Unload existing superchunks we don't need anymore
        LocalVector<WorldManSuperchunk>::Iterator elem = superchunks.begin();
        for (int i = 0; i < superchunks.size(); i++) {
            const Vector2i superchunk_pos = superchunks[i].position;
            if (!visible_superchunks.has(superchunk_pos)) {
                unload_superchunk(superchunk_pos);
                superchunks.remove_at_unordered(i);
                i--;
                continue;
            }
        }
    }
    int created_superchunks = 0;
    const uint64_t start = Time::get_singleton()->get_ticks_usec();
    // Now, create the new superchunks
    {
        for (const Vector2i superchunk_pos : visible_superchunks) {

            if (find_superchunk(superchunk_pos) == nullptr) {
                created_superchunks++;
                create_superchunk(superchunk_pos);
            }
        }
    }

    current_task.created_superchunks = created_superchunks;
    current_task.task_start_time_usec = Time::get_singleton()->get_ticks_usec();
    current_task.camera_position = Vector2(p_camera_position.x, p_camera_position.z);
    camera_position = current_task.camera_position;

    // Update time, onto another thread!
    WorkerThreadPool *wp = WorkerThreadPool::get_singleton();
    current_task.task_id = wp->add_native_group_task(&WorldMan::_update_task, this, superchunks.size());
    update_state = UpdateState::UPDATING;
}

const WorldMan::WorldManSuperchunk *WorldMan::find_superchunk(const Vector2i &p_pos) const {
    for (const WorldManSuperchunk &superchunk : superchunks) {
        if (superchunk.position == p_pos) {
            return &superchunk;
        }
    }
    return nullptr;
}

WorldMan::WorldManSuperchunk *WorldMan::find_superchunk(const Vector2i &p_pos) {
    for (WorldManSuperchunk &superchunk : superchunks) {
        if (superchunk.position == p_pos) {
            return &superchunk;
        }
    }
    return nullptr;
}

Rect2i WorldMan::get_superchunk_world_rect(Vector2i p_superchunk) const {
	return Rect2(p_superchunk * SUPERCHUNK_SIZE, Vector2i(SUPERCHUNK_SIZE, SUPERCHUNK_SIZE));
}

WorldMan *WorldMan::create(Ref<ShaderMaterial> p_terrain_shader) {
    DEV_ASSERT(p_terrain_shader.is_valid());
	WorldMan *wm = memnew(WorldMan);
    wm->terrain_shader = p_terrain_shader;
    wm->chunk_mesh = LODMesh::generate_mesh(17);
    wm->chunk_mesh->surface_set_material(0, wm->terrain_shader);
    return wm;
}

void WorldMan::_ready() {
    Window *window = memnew(Window);

    debugger = memnew(WorldManDebugger);
    debugger->set_worldman(this);

    add_child(window);
    window->set_size(Vector2i(512, 512));

    window->add_child(debugger);
    debugger->set_anchors_and_offsets_preset(Control::LayoutPreset::PRESET_FULL_RECT);
}

double WorldMan::bilinearly_sample_image_single_channel(Ref<Image> p_image, int p_channel_idx, const Vector2 &p_uv) {
    const Size2i image_size = p_image->get_size();

    // Convert to heightmap pixel coordinates
    double u = p_uv.x * (image_size.x - 1);
    double v = p_uv.y * (image_size.y - 1);

    // Bilinear interpolation for smoothness
    int u0 = (int)floor(u);
    int v0 = (int)floor(v);
    int u1 = MIN(u0 + 1, image_size.x - 1);
    int v1 = MIN(v0 + 1, image_size.y - 1);
    double u_frac = u - u0;
    double v_frac = v - v0;

    if (u0 >= image_size.x || v0 >= image_size.y) {
        return 0.0f;
    }

    // Sample values from the channel
    double h00 = p_image->get_pixel(u0, v0)[p_channel_idx];
    double h10 = p_image->get_pixel(u1, v0)[p_channel_idx];
    double h01 = p_image->get_pixel(u0, v1)[p_channel_idx];
    double h11 = p_image->get_pixel(u1, v1)[p_channel_idx];

    // Bilinear interpolation
    double out = h00 * (1 - u_frac) * (1 - v_frac) +
                    h10 * u_frac * (1 - v_frac) +
                    h01 * (1 - u_frac) * v_frac +
                    h11 * u_frac * v_frac;
    return out;
}

WorldMan::WorldMan() {
	noise.instantiate();
    noise->set_frequency(0.01);

    Ref<Image> img = Image::create_empty(1, 1, false, Image::Format::FORMAT_RF);
    img->set_pixel(0, 0, Color(1000.0, 1000.0, 1000.0, 1000.0));
    empty_road_texture = ImageTexture::create_from_image(img);
}

WorldMan::~WorldMan() {
    WorkerThreadPool *wp = WorkerThreadPool::get_singleton();
    wp->wait_for_group_task_completion(current_task.task_id);
    for (WorldManSuperchunk &sc : superchunks) {
        unload_superchunk(sc.position);
    }
}
