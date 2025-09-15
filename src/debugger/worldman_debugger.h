#pragma once

#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/classes/panel_container.hpp"
#include "godot_cpp/templates/local_vector.hpp"
#include "superchunk_map.h"
#include "superchunk_debugger.h"

using namespace godot;

class WorldMan;

class WorldManDebugger : public PanelContainer {
    GDCLASS(WorldManDebugger, PanelContainer);
    WorldMan *wm = nullptr;

    float draw_range = 1000.0f;
    void _on_draw_range_changed(float p_new_draw_range);
    void _on_superchunk_selected(Vector2i p_superchunk);
    SuperchunkMap *map = nullptr;
    Window *superchunk_debugger_window = nullptr;
    SuperchunkDebugger *superchunk_debugger = nullptr;

public:
    static void _bind_methods();
    virtual void _ready() override;
    virtual void _draw() override;

    void set_worldman(WorldMan *p_worldman);
};