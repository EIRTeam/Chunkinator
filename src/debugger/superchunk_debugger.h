#pragma once

#include "worldman.h"
#include "godot_cpp/classes/control.hpp"

class SuperchunkDebugger : public Control {
    GDCLASS(SuperchunkDebugger, Control);

    WorldMan *wm = nullptr;
    Vector2i superchunk;
public:
    static void _bind_methods();
    void set_worldman(WorldMan *p_worldman);
    void set_superchunk(Vector2i p_superchunk);
    virtual void _draw() override;
};