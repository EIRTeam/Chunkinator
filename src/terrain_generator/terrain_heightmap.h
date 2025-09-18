#pragma once

#include "chunkinator/chunkinator_chunk.h"
#include "chunkinator/chunkinator_layer.h"
#include "godot_cpp/classes/fast_noise_lite.hpp"
#include "godot_cpp/classes/image.hpp"

using namespace godot;

class TerrainHeightmapLayer : public ChunkinatorLayer {
    Ref<FastNoiseLite> noise;
public:
    virtual int get_chunk_size() const override;
    virtual Ref<ChunkinatorChunk> instantiate_chunk() override;
    double sample_noise(const Vector2 &p_world_position) const;
    TerrainHeightmapLayer();
};

class TerrainHeightmapChunk : public ChunkinatorChunk {
    Ref<Image> heightmap;
    TerrainHeightmapLayer *layer;
    Ref<Texture2D> debug_texture;
    static constexpr int DATA_SIZE = 64;
public:
    virtual void generate() override;
    virtual void debug_draw(ChunkinatorDebugDrawer *p_debug_drawer) const override;
    friend class TerrainHeightmapLayer;
};