#include "game_rules_laniakea.h"
#include "gdextension_interface.h"

CVar GameRulesLaniakea::rifle_max_ammo_cvar = CVar::create_variable("sk_max_rifle", GDEXTENSION_VARIANT_TYPE_INT, 120, "Maximum rifle ammo", PROPERTY_HINT_NONE, "");
CVar GameRulesLaniakea::rifle_player_damage_cvar = CVar::create_variable("sk_plr_dmg_rifle", GDEXTENSION_VARIANT_TYPE_INT, 5, "Rifle damage to player", PROPERTY_HINT_NONE, "");
CVar GameRulesLaniakea::rifle_npc_damage_cvar = CVar::create_variable("sk_npc_dmg_rifle", GDEXTENSION_VARIANT_TYPE_INT, 5, "Rifle damage to NPCs", PROPERTY_HINT_NONE, "");

void GameRulesLaniakea::initialize() {
    register_ammo_type(RIFLE_AMMO_TYPE, "rifle", "sk_max_rifle", "sk_plr_dmg_rifle", "sk_npc_dmg_rifle");
    GameRules::initialize();
}
