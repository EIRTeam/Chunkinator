#pragma once

#include "console/cvar.h"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/templates/hash_map.hpp"

using namespace godot;

class GameRules : public Object {
    GDCLASS(GameRules, Object);

    struct AmmoTypeInfo {
        StringName ammo_name;
        CVar *ammo_max_cvar;
        CVar *ammo_player_damage_cvar;
        CVar *ammo_npc_damage_cvar;
    };
    
    HashMap<int, AmmoTypeInfo> ammo_types;
    static void _bind_methods() {}
public:
    void register_ammo_type(int p_id, const StringName p_name, const StringName p_ammo_max_cvar, const StringName p_player_damage_cvar, const StringName p_npc_damage_cvar);
    virtual void initialize();
};