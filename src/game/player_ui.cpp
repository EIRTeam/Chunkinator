#include "player_ui.h"
#include "godot_cpp/classes/canvas_item.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/camera3d.hpp"
#include "godot_cpp/classes/node2d.hpp"

void PlayerUI::_ready() {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }
    crosshairs_container = get_node<Control>("%Crosshairs");
    occlusion_crosshairs_container = get_node<Control>("%OcclusionCrosshairs");
}

void PlayerUI::update(Camera3D *p_camera, float p_delta) {
    
}

void PlayerUI::update_crosshair(int p_slot, Camera3D *p_camera, bool p_show_crosshair, bool p_occluded, Vector3 p_occlusion_position) {
    ERR_FAIL_INDEX(p_slot, crosshairs_container->get_child_count());
    ERR_FAIL_INDEX(p_slot, occlusion_crosshairs_container->get_child_count());
    
    Node2D *crosshair = Object::cast_to<Node2D>(crosshairs_container->get_child(p_slot));
    Node2D *occlusion_crosshair = Object::cast_to<Node2D>(occlusion_crosshairs_container->get_child(p_slot));

    ERR_FAIL_COND(crosshair == nullptr);
    ERR_FAIL_COND(occlusion_crosshair == nullptr);

    crosshair->set_visible(p_show_crosshair);
    occlusion_crosshair->set_visible(p_show_crosshair && p_occluded && !p_camera->is_position_behind(p_occlusion_position));
    if (occlusion_crosshair->is_visible()) {
        occlusion_crosshair->set_global_position(p_camera->unproject_position(p_occlusion_position));
    }
}
