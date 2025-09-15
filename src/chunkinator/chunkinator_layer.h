#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/templates/local_vector.hpp"

using namespace godot;

class ChunkinatorChunk;
class Chunkinator;

class ChunkinatorLayer : public RefCounted {
    LocalVector<Ref<ChunkinatorChunk>> chunks;
    LocalVector<StringName> parents;
    LocalVector<StringName> children;
    StringName name;
    Rect2i rect_to_generate;
    int dag_level = 0;
public:
    virtual int get_chunk_size() const = 0;
    virtual Ref<ChunkinatorChunk> instantiate_chunk() = 0;

    friend class Chunkinator;
};
