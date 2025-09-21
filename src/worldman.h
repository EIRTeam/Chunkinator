#pragma once

#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/classes/shader.hpp"
#include "godot_cpp/classes/shader_material.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/templates/hash_map.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/worker_thread_pool.hpp"
#include "godot_cpp/classes/fast_noise_lite.hpp"
#include "godot_cpp/classes/fast_noise_lite.hpp"
#include "godot_cpp/classes/curve2d.hpp"
#include "quadtree.h"

#include <optional>

namespace godot {
class MeshInstance3D;
class StaticBody3D;
}

using namespace godot;

class WorldManDebugger;
class SuperchunkMap;
class SuperchunkDebugger;

class WorldMan : public Node3D {
    GDCLASS(WorldMan, Node3D);
private:
    static constexpr int SUPERCHUNK_RADIUS = 6;
    static constexpr int SUPERCHUNK_SIZE = 4096;
    static constexpr int SUPERCHUNK_MAX_DEPTH = 5;
    static constexpr int CHUNK_MIN_DEPTH_FOR_COLLISION = 4;
    static constexpr int ROAD_SPLINE_COUNT_PER_SUPERCHUNK = 14;
    static constexpr float ROAD_WIDTH = 6.0f;
    static constexpr float ROAD_SKIRT = 3.0f;
    struct WorldManChunk {
        enum ChunkState {
            NEEDS_MESH_INSTANCE_INSTANTIATION,
            NEEDS_UNLOADING,
            OK
        };
        ChunkState state = ChunkState::NEEDS_MESH_INSTANCE_INSTANTIATION;
        Rect2i rect;
        int depth = 0;
        BitField<LODMesh::LODMask> lod_mask = 0;
        MeshInstance3D *mesh_instance = nullptr;
        StaticBody3D *collision_body = nullptr;
        Ref<ConcavePolygonShape3D> collision_shape;
    };

    struct WorldManSuperchunkRoadInfo {
        Ref<Curve2D> road_spline;

        // This texture includes the 2D positions and height of each road spline position in order
        Ref<Texture2D> road_spline_texture;
        Vector3 road_spline_positions[ROAD_SPLINE_COUNT_PER_SUPERCHUNK+2];
    };

    struct WorldManSuperchunk {
        enum State {
            CREATING,
            NEEDS_INSTANCING,
            IDLE
        };

        State state = State::CREATING;
        Ref<QuadTree> quadtree;
        Ref<Image> base_normal_image;
        Ref<ImageTexture> base_normal_texture;
        Ref<ShaderMaterial> material;
        Ref<ArrayMesh> mesh;
        Vector2i position;
        LocalVector<WorldManChunk> chunks;
        std::optional<WorldManSuperchunkRoadInfo> road_info;
    };
    static void _update_task(void *p_userdata, uint32_t p_element_idx);
    Ref<ConcavePolygonShape3D> generate_collision_mesh_for_rect(const WorldMan::WorldManSuperchunk &p_superchunk, const Rect2i &p_local_rect) const;
    _FORCE_INLINE_ double sample_superchunk_noise(const WorldMan::WorldManSuperchunk &p_superchunk, const Vector2 &p_local_position) const;
    void _update_task_finished();
    void _create_superchunk_roads(WorldManSuperchunk &p_superchunk);
    Ref<FastNoiseLite> noise;
    WorldManDebugger *debugger = nullptr;
    Vector2 camera_position;
    void apply_road_height(const WorldManSuperchunk &p_superchunk, Vector2 p_local_position, float &p_height);
public:
    struct CurrentTask {
        WorkerThreadPool::GroupID task_id = -1;
        int task_start_time_usec = 0;
        int created_superchunks = 0;
        Vector2 camera_position;
    } current_task;

    enum UpdateState {
        IDLE,
        UPDATING
    };

    UpdateState update_state = UpdateState::IDLE;

    LocalVector<WorldManSuperchunk> superchunks;
    Ref<ShaderMaterial> terrain_shader;
    Ref<ArrayMesh> chunk_mesh;
    Ref<ImageTexture> empty_road_texture;

    static void _bind_methods();
    void unload_superchunk(Vector2i p_superchunk);
    void create_superchunk(Vector2i p_superchunk);

    void unload_chunk(WorldManSuperchunk &p_superchunk, int p_chunk_idx);
    TypedArray<Vector2i> get_visible_superchunks_bind() const;
    void update(Vector3 p_camera_position);
    const WorldManSuperchunk *find_superchunk(const Vector2i &p_pos) const;
    WorldManSuperchunk *find_superchunk(const Vector2i &p_pos);
    Rect2i get_superchunk_world_rect(Vector2i p_superchunk) const;
    static WorldMan *create(Ref<ShaderMaterial> p_terrain_shader);
    virtual void _ready() override;

    static double bilinearly_sample_image_single_channel(Ref<Image> p_image, int p_channel_idx, const Vector2 &p_uv);
    WorldMan();
    ~WorldMan();
    
    friend class WorldManDebugger;
    friend class SuperchunkMap;
    friend class SuperchunkDebugger;
};