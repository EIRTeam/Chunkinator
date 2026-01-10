#pragma once

#include "godot_cpp/classes/control.hpp"

using namespace godot;

namespace godot {
class Camera3D;
}

class PlayerUI : public Control {
    GDCLASS(PlayerUI, Control);

    Control *crosshairs_container = nullptr;
    Control *occlusion_crosshairs_container = nullptr;
public:
    static void _bind_methods() {};
    virtual void _ready() override;
    void update(Camera3D *p_camera, float p_delta);
    void update_crosshair(int p_slot, Camera3D *p_camera, bool p_show_crosshair, bool p_occluded, Vector3 occlusion_position);
};