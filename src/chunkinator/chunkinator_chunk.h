#pragma once

#include "godot_cpp/classes/ref_counted.hpp"

using namespace godot;

class ChunkinatorChunk : public RefCounted {
    Rect2i bounds;
public:
    Rect2i get_chunk_bounds() const;
    virtual void generate() {}
};
