#include "player_character_milk.h"

void PlayerCharacterMilkCarried::_physics_process(double p_delta) {
    
}

void PlayerCharacterMilkCarried::update_carried(double p_delta) {
    if (player_attached_to == nullptr) {
        return;
    }

    model->set_global_transform(player_attached_to->get_milk_attachment_transform());
}

void PlayerCharacterMilkCarried::attach_to_player(PlayerCharacter *p_player) {
    player_attached_to = p_player;
    movement.set_enabled(false);
    animation->set_hanging(true);
}
