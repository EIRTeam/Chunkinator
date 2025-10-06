#pragma once

#include "chunkinator/chunkinator.h"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/texture2d_array.hpp"
#include "godot_cpp/classes/texture2drd.hpp"
#include "godot_cpp/variant/rid.hpp"
#include "godot_cpp/variant/vector2i.hpp"
#include "terrain_generator/terrain_chunk_node.h"
#include "terrain_generator/terrain_heightmap_combine_layer.h"
#include "terrain_generator/terrain_roads.h"
#include "terrain_generator/terrain_settings.h"
#include "terrain_generator/texture_array_queue.h"

using namespace godot;

class TerrainSpatialPaging {
    RID texture_id;

    PackedByteArray indices;
    Ref<Texture2DRD> texture;
    int page_size = 0;
public:
    void initialize(int p_size);
    void set_index(Vector2i p_position, int p_value);
    void upload_page();
    void clear(int p_clear_value);
    Ref<Texture2D> get_texture() const;
    ~TerrainSpatialPaging();
};


class TerrainManager : public Node3D {
    GDCLASS(TerrainManager, Node3D);
    Ref<Chunkinator> chunkinator;
    struct TerrainSuperchunk {
        TerrainChunkNode *node = nullptr;
    };

    HashMap<Vector2i, TerrainSuperchunk> superchunks;
    void _on_generation_completed();

    Ref<Mesh> plane_mesh;
    Ref<TerrainSettings> settings;
    Vector3 camera_position;

    TerrainSpatialPaging terrain_heightmap_spatial_page;
    TextureArrayQueue terrain_heightmap_data_array;
    Vector2i spatial_page_origin;
public:
    static void _bind_methods();
    void _update_spatial_page();
    void set_chunkinator(Ref<Chunkinator> p_chunkinator);
    void _unload_superchunk(const Vector2i &p_idx);
    void set_camera_position(Vector3 p_camera_position);
    void set_terrain_settings(const Ref<TerrainSettings> &p_settings);
    Ref<TerrainFinalCombineLayer> get_terrain_layer() const;
    Ref<TerrainRoadConnectionLayer> get_road_layer() const;
    void update();
    void _notification(int p_what);
};