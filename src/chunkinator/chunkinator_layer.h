#pragma once

#include "chunkinator/chunkinator_bounds.h"
#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/templates/local_vector.hpp"

using namespace godot;

class ChunkinatorChunk;
class Chunkinator;

class ChunkinatorLayer : public RefCounted {
    LocalVector<Ref<ChunkinatorChunk>> chunks;
    LocalVector<Ref<ChunkinatorChunk>> generating_chunks;
    LocalVector<StringName> parents;
    LocalVector<StringName> children;
    StringName name;
    Rect2i world_rect_to_generate;
    ChunkinatorBounds chunk_idx_bounds_to_generate;
    int dag_level = 0;
public:
    virtual int get_chunk_size() const = 0;
    virtual Ref<ChunkinatorChunk> instantiate_chunk() = 0;
    Ref<ChunkinatorChunk> get_chunk_by_index(Vector2i p_chunk_idx) const;

    friend class Chunkinator;
};
