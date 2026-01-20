#pragma once

#include "godot_cpp/classes/control.hpp"
#include "base_character.h"

using namespace godot;

namespace godot {
class Camera3D;
}

class PlayerCharacter;
class ItemSelectorUI;

class PlayerUI : public Control {
    GDCLASS(PlayerUI, Control);

    Control *crosshairs_container = nullptr;
    Control *occlusion_crosshairs_container = nullptr;
    ItemSelectorUI *current_selector_ui = nullptr;
public:
    void notify_equip_item_requested(StringName p_item_name, int p_slot);
    void notify_unequip_item_requested(int p_slot);
    static void _bind_methods();
    virtual void _ready() override;
    void _on_equip_weapon_requested(StringName p_weapon, BaseCharacter::WeaponSlot p_slot);
    void update(PlayerCharacter *p_player, float p_delta);
    void update_crosshair(int p_slot, Camera3D *p_camera, bool p_show_crosshair, bool p_occluded, Vector3 occlusion_position);
    void show_weapon_selector(PlayerCharacter *p_character);
};