#pragma once

#include "chunkinator/chunkinator.h"
#include "godot_cpp/classes/node3d.hpp"
#include "terrain_generator/terrain_chunk_node.h"
#include "terrain_generator/terrain_heightmap_combine_layer.h"
#include "terrain_generator/terrain_roads.h"
#include "terrain_generator/terrain_settings.h"

using namespace godot;

class TerrainManager : public Node3D {
    GDCLASS(TerrainManager, Node3D);
    Ref<Chunkinator> chunkinator;
    struct TerrainSuperchunk {
        TerrainChunkNode *node = nullptr;
    };

    HashMap<Vector2i, TerrainSuperchunk> superchunks;
    void _on_generation_completed();
    
    const int mesh_quality = 17;
    Ref<Mesh> plane_mesh;
    Ref<TerrainSettings> settings;
    Vector3 camera_position;
public:
    static void _bind_methods();
    void set_chunkinator(Ref<Chunkinator> p_chunkinator);
    void _unload_superchunk(const Vector2i &p_idx);
    void set_camera_position(Vector3 p_camera_position);
    void set_terrain_settings(const Ref<TerrainSettings> &p_settings);
    Ref<TerrainFinalCombineLayer> get_terrain_layer() const;
    Ref<TerrainRoadConnectionLayer> get_road_layer() const;
    void update();
    void _notification(int p_what);
};