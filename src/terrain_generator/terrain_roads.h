#pragma once

#include "chunkinator/chunkinator_chunk.h"
#include "chunkinator/chunkinator_layer.h"
#include "godot_cpp/classes/fast_noise_lite.hpp"
#include "godot_cpp/classes/image.hpp"

class TerrainRoadConnectionLayer : public ChunkinatorLayer {
    virtual int get_chunk_size() const override;
    virtual Ref<ChunkinatorChunk> instantiate_chunk() override;
public:
    LocalVector<Vector2> get_points_in_radius(Vector2 p_position, float p_radius) const;
    LocalVector<Vector2> get_points_in_bounds(Rect2 p_bounds) const;
};

class TerrainRoadConnectionChunk : public ChunkinatorChunk {
    TerrainRoadConnectionLayer *layer = nullptr;
    struct Connection {
        Vector2 from;
        Vector2 to;
    };
    LocalVector<Connection> connections;
    const float connection_radius = 6500.0f;
    const int maximum_connections = 1;
    virtual void generate() override;
	virtual void debug_draw(ChunkinatorDebugDrawer *p_debug_drawer) const;
	friend class TerrainRoadConnectionLayer;
};

class TerrainHeightmapRoadFlatteningLayer : public ChunkinatorLayer {
    
};