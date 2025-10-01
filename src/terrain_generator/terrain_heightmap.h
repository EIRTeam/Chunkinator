#pragma once

#include "baked_map.h"
#include "chunkinator/chunkinator_chunk.h"
#include "chunkinator/chunkinator_layer.h"
#include "godot_cpp/classes/fast_noise_lite.hpp"
#include "godot_cpp/classes/image.hpp"
#include "terrain_generator/terrain_settings.h"

using namespace godot;

class TerrainHeightmapLayer;

class TerrainHeightmapChunk : public ChunkinatorChunk {
    Ref<Image> heightmap;
    BakedMap baked_heightmap;
    TerrainHeightmapLayer *layer;
    Ref<Texture2D> debug_texture;
    static constexpr int DATA_SIZE = 128;
public:
    virtual void generate() override;
    virtual void debug_draw(ChunkinatorDebugDrawer *p_debug_drawer) const override;
    float sample_height(const Vector2 &p_world_position) const;
    BakedMap *get_baked_heightmap() const;
    friend class TerrainHeightmapLayer;
};

class TerrainHeightmapLayer : public ChunkinatorLayer {
    Ref<TerrainSettings> settings;
public:
    virtual int get_chunk_size() const override;
    virtual Ref<ChunkinatorChunk> instantiate_chunk() override;
    double sample_noise(const Vector2 &p_world_position) const;
    _FORCE_INLINE_ float sample_height(const Vector2 &p_world_position) const { 
        Ref<TerrainHeightmapChunk> chunk = get_chunk_in_position(p_world_position);
        return chunk->sample_height(p_world_position);
    }
    TerrainHeightmapLayer();
};
