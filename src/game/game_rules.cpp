#include "game_rules.h"
#include "console/console_system.h"

void GameRules::register_ammo_type(int p_id, const StringName p_name, const StringName p_ammo_max_cvar, const StringName p_player_damage_cvar, const StringName p_npc_damage_cvar) {
    
    ConsoleSystem *cs = ConsoleSystem::get_singleton();

    DEV_ASSERT(cs->get_cvar(p_ammo_max_cvar) != nullptr);
    DEV_ASSERT(cs->get_cvar(p_player_damage_cvar) != nullptr);
    DEV_ASSERT(cs->get_cvar(p_npc_damage_cvar) != nullptr);
    DEV_ASSERT(!ammo_types.has(p_id));

    AmmoTypeInfo *type_info = memnew(AmmoTypeInfo);

    ammo_types.insert(p_id, {
        .ammo_name = p_name,
        .ammo_max_cvar = cs->get_cvar(p_ammo_max_cvar),
        .ammo_player_damage_cvar = cs->get_cvar(p_player_damage_cvar),
        .ammo_npc_damage_cvar = cs->get_cvar(p_npc_damage_cvar)
    });
}

void GameRules::initialize() {
}
