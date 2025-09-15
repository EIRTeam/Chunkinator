#pragma once

#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/input_event.hpp"
#include "worldman.h"

using namespace godot;

class SuperchunkMap : public Control {
    GDCLASS(SuperchunkMap, Control);
    WorldMan *wm = nullptr;
    float draw_range = 1000.0f;
    struct SuperchunkDrawData {
        Vector2i superchunk_grid_position;
        Rect2i local_rect;
    };
    LocalVector<SuperchunkDrawData> superchunk_draw_datas = {};
    
public:
    static void _bind_methods();
    void set_draw_range(float p_draw_range);
    virtual void _draw() override;
    void set_worldman(WorldMan *p_worldman);
	virtual void _gui_input(const Ref<InputEvent> &p_event);
    SuperchunkMap();
};
