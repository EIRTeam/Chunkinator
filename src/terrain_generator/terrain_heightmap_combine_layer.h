#pragma once

#include "chunkinator/chunkinator_chunk.h"
#include "chunkinator/chunkinator_layer.h"
#include "godot_cpp/classes/fast_noise_lite.hpp"
#include "godot_cpp/classes/image.hpp"
#include "terrain_generator/terrain_heightmap.h"
#include "terrain_generator/terrain_roads.h"

using namespace godot;

class TerrainFinalCombineLayer : public ChunkinatorLayer {
    Ref<TerrainRoadConnectionLayer> road_connection_layer;
    Ref<TerrainHeightmapLayer> heightmap_layer;
public:
    virtual int get_chunk_size() const override;
    virtual Ref<ChunkinatorChunk> instantiate_chunk() override;
    double sample_height(Vector2 p_world_position) const;
    Ref<TerrainHeightmapLayer> get_heightmap_layer() const;
    Ref<TerrainRoadConnectionLayer> get_road_connection_layer() const;
};

class TerrainFinalCombineChunk : public ChunkinatorChunk {
    Ref<Image> height_map;
    Ref<Image> road_sdf;
    Ref<ImageTexture> debug_draw_texture;
    Ref<ImageTexture> debug_road_sdf_texture;
    TerrainFinalCombineLayer *layer = nullptr;
    const int heightmap_size = 64;
    const int road_sdf_size = 64;
public:
    virtual void generate();
    virtual void debug_draw(ChunkinatorDebugDrawer *p_debug_drawer) const;
    Ref<Image> get_height_map() const;
    Ref<Image> get_road_sdf() const;
    friend class TerrainFinalCombineLayer;
};