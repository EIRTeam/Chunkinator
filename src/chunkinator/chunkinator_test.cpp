#include "chunkinator_test.h"
#include "chunkinator/chunkinator_debugger.h"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/input_event_key.hpp"
#include "godot_cpp/classes/window.hpp"
#include "terrain_generator/random_point_scatter.h"
#include "terrain_generator/terrain_heightmap.h"
#include "terrain_generator/terrain_roads.h"

void ChunkinatorTest::_input(const Ref<InputEvent> &p_event) {
    if (Ref<InputEventKey> ev = p_event; ev.is_valid()) {
        if (ev->is_pressed() && !ev->is_echo() && ev->get_keycode() == godot::KEY_F2) {
            chunkinator->set_generation_rect(Rect2(-1000.0, -1000.0, 20000, 20000));
            chunkinator->generate();
        }
    }
}

void ChunkinatorTest::_bind_methods() {
    
}

void ChunkinatorTest::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_READY: {
            chunkinator.instantiate();
            Ref<TerrainHeightmapLayer> A_Layer;
            Ref<RandomPointLayer> B_Layer;
            Ref<TerrainRoadConnectionLayer> C_Layer;
            Ref<ChunkinatorTestLayer> D_Layer;
            
            A_Layer.instantiate();
            B_Layer.instantiate();
            C_Layer.instantiate();
            D_Layer.instantiate();
            
            const StringName a_layer_name = "Heightmap";
            const StringName b_layer_name = "Road Random Points";
            const StringName c_layer_name = "Road Connections";

            chunkinator->insert_layer(a_layer_name, A_Layer);
            chunkinator->insert_layer(b_layer_name, B_Layer);
            chunkinator->insert_layer(c_layer_name, C_Layer);
            //chunkinator->insert_layer("C Layer", C_Layer);
            //chunkinator->insert_layer("D Layer", D_Layer);

            chunkinator->add_layer_dependency(a_layer_name, c_layer_name, Vector2i(50, 50));
            chunkinator->add_layer_dependency(b_layer_name, c_layer_name, Vector2i(3000, 3000));
            //chunkinator->add_layer_dependency(a_layer_name, "C Layer", Vector2i());
            //chunkinator->add_layer_dependency(b_layer_name, "D Layer", Vector2i());
            //chunkinator->add_layer_dependency("C Layer", "D Layer", Vector2i());

            chunkinator->build();
            chunkinator->set_generation_rect(Rect2(-10000, -10000, 20000, 20000));
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
}

void ChunkinatorTestChunk::debug_draw(ChunkinatorDebugDrawer *p_debug_drawer) const {
    if (get_chunk_index() == Vector2i()) {
        p_debug_drawer->draw_circle(Rect2(get_chunk_bounds()).get_center());
    }
}
