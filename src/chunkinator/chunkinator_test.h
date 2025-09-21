#pragma once

#include "chunkinator.h"
#include "chunkinator/chunkinator_chunk.h"
#include "chunkinator/chunkinator_debugger.h"
#include "chunkinator/chunkinator_layer.h"
#include "godot_cpp/classes/node3d.hpp"

class ChunkinatorTestChunk : public ChunkinatorChunk {
    void test();
    virtual void generate() override;
    virtual void debug_draw(ChunkinatorDebugDrawer *p_debug_drawer) const override;
};

class ChunkinatorTestLayer : public ChunkinatorLayer {
public:
    int get_chunk_size() const override;
    Ref<ChunkinatorChunk> instantiate_chunk() override;
};

class ChunkinatorTest : public Node3D {
    GDCLASS(ChunkinatorTest, Node3D);
    ChunkinatorDebugger *debugger = nullptr;
public:
    virtual void _input(const Ref<InputEvent> &p_event) override;
    Ref<Chunkinator> chunkinator;
    static void _bind_methods();
    void _notification(int p_what);
    ChunkinatorTest();
};