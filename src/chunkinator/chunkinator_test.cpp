#include "chunkinator_test.h"
#include "chunkinator/chunkinator_debugger.h"
#include "godot_cpp/classes/window.hpp"
#include "terrain_generator/terrain_heightmap.h"

void ChunkinatorTest::_bind_methods() {
    
}

void ChunkinatorTest::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_READY: {
            chunkinator.instantiate();
            Ref<TerrainHeightmapLayer> A_Layer;
            Ref<ChunkinatorTestLayer> B_Layer;
            Ref<ChunkinatorTestLayer> C_Layer;
            Ref<ChunkinatorTestLayer> D_Layer;
            
            A_Layer.instantiate();
            B_Layer.instantiate();
            C_Layer.instantiate();
            D_Layer.instantiate();
            
            chunkinator->insert_layer("A Layer", A_Layer);
            //chunkinator->insert_layer("B Layer", B_Layer);
            //chunkinator->insert_layer("C Layer", C_Layer);
            //chunkinator->insert_layer("D Layer", D_Layer);

            //chunkinator->add_layer_dependency("A Layer", "B Layer", Vector2i());
            //chunkinator->add_layer_dependency("A Layer", "C Layer", Vector2i());
            //chunkinator->add_layer_dependency("B Layer", "D Layer", Vector2i());
            //chunkinator->add_layer_dependency("C Layer", "D Layer", Vector2i());

            chunkinator->build();
            chunkinator->generate();
            Window *w = memnew(Window);
            add_child(w);
            w->hide();
            w->set_force_native(true);
            w->show();
            w->set_size(Vector2(512, 512));
            debugger = memnew(ChunkinatorDebugger);
            w->add_child(debugger);
            debugger->set_chunkinator(chunkinator);
            debugger->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
        } break;
        case NOTIFICATION_PROCESS: {
            chunkinator->process_generation();
        } break;
    }
}

ChunkinatorTest::ChunkinatorTest() {
    set_process(true);
}

int ChunkinatorTestLayer::get_chunk_size() const {
    return 4096;
}

Ref<ChunkinatorChunk> ChunkinatorTestLayer::instantiate_chunk() {
    Ref<ChunkinatorTestChunk> test_chunk;
    test_chunk.instantiate();
    Ref<ChunkinatorChunk> pc = test_chunk;
    return pc;
}

void ChunkinatorTestChunk::test() {
    
}

void ChunkinatorTestChunk::generate() {
    print_line("CHUNK!!", get_chunk_bounds(), get_chunk_index());
}
