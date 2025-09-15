#include "worldman_debugger.h"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/classes/v_box_container.hpp"
#include "godot_cpp/classes/h_slider.hpp"
#include "godot_cpp/classes/spin_box.hpp"
#include "godot_cpp/classes/camera3d.hpp"
#include "godot_cpp/classes/viewport.hpp"
#include "godot_cpp/classes/window.hpp"
#include "worldman.h"

void WorldManDebugger::_on_draw_range_changed(float p_new_draw_range) {
    draw_range = p_new_draw_range;
    map->set_draw_range(p_new_draw_range);
}

void WorldManDebugger::_on_superchunk_selected(Vector2i p_superchunk) {
    superchunk_debugger->set_superchunk(p_superchunk);
    superchunk_debugger_window->show();
}

void WorldManDebugger::_bind_methods() {
}

void WorldManDebugger::_ready() {
    VBoxContainer *vbox = memnew(VBoxContainer);
    add_child(vbox);

    map = memnew(SuperchunkMap);
    map->set_worldman(wm);
    map->connect("superchunk_selected", callable_mp(this, &WorldManDebugger::_on_superchunk_selected));

    vbox->add_child(map);
    map->set_v_size_flags(Control::SizeFlags::SIZE_EXPAND_FILL);

    HBoxContainer *bottom_bar_container = memnew(HBoxContainer);
    vbox->add_child(bottom_bar_container);
    
    HSlider *radius_slider = memnew(HSlider);

    radius_slider->set_h_size_flags(Control::SizeFlags::SIZE_EXPAND_FILL);

    SpinBox *radius_spinbox = memnew(SpinBox);
    
    bottom_bar_container->add_child(radius_slider);
    bottom_bar_container->add_child(radius_spinbox);

    radius_spinbox->share(radius_slider);

    radius_slider->set_min(1000.0f);
    radius_slider->set_max(50000.0f);

    radius_slider->connect(StringName("value_changed"), callable_mp(this, &WorldManDebugger::_on_draw_range_changed));

    vbox->set_anchors_and_offsets_preset(Control::LayoutPreset::PRESET_FULL_RECT);

    superchunk_debugger_window = memnew(Window);
    superchunk_debugger_window->hide();
    add_child(superchunk_debugger_window);

    superchunk_debugger = memnew(SuperchunkDebugger);
    superchunk_debugger->set_worldman(wm);
    superchunk_debugger->set_anchors_and_offsets_preset(Control::LayoutPreset::PRESET_FULL_RECT);
    superchunk_debugger_window->add_child(superchunk_debugger);
}

void WorldManDebugger::_draw() {

}

void WorldManDebugger::set_worldman(WorldMan *p_worldman) {
    wm = p_worldman;
}