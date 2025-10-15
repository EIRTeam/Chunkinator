#pragma once

#include "chunkinator/chunk_spawner.h"

class ScatterManager : public ChunkSpawner {
    HashMap<Vector2i, Node3D*> scatter_chunks;

    virtual void spawn_chunk(const Vector2i &p_chunk);
    virtual void unload_chunk(const Vector2i &p_chunk);
    virtual StringName get_layer_name() const {return "Trees";};
};