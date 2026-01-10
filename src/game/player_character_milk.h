#pragma once

#include "game/base_character.h"
#include "game/player_character.h"
class PlayerCharacterMilkCarried : public BaseCharacter {
    GDCLASS(PlayerCharacterMilkCarried, BaseCharacter);

public:

    static void _bind_methods() {}
    PlayerCharacter *player_attached_to = nullptr;

    virtual void _physics_process(double p_delta) override;
    void update_carried(double p_delta);
    void attach_to_player(PlayerCharacter *p_player);
};