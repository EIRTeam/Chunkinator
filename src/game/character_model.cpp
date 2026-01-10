#include "character_model.h"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/property_info.hpp"
#include "godot_cpp/variant/quaternion.hpp"
#include "animation/hip_rotator_modifier.h"

void CharacterModel::_bind_methods() {
    MAKE_BIND_NODE(CharacterModel, skeleton, Skeleton3D);
    MAKE_BIND_NODE(CharacterModel, animation_tree, AnimationTree);
    MAKE_BIND_NODE(CharacterModel, milk_attachment_point, Node3D);
    MAKE_BIND_NODE(CharacterModel, milk_hip_target, Node3D);
    MAKE_BIND_NODE(CharacterModel, firing_position_node, Node3D);
    MAKE_BIND_NODE(CharacterModel, hand_attachment_node, Node3D);
    MAKE_BIND_NODE(CharacterModel, hip_rotator, HipRotatorModifier3D);
}

void CharacterModel::update(float p_delta) {
    if (!target_facing_direction.is_normalized()) {
        return;
    }
    facing_spring.update(target_facing_direction.normalized(), p_delta);

    set_global_basis(facing_spring.get_value());
}

void CharacterModel::set_target_facing_direction(Vector3 p_facing_direction) {
    DEV_ASSERT(p_facing_direction.is_normalized());
    target_facing_direction = Basis::looking_at(p_facing_direction).get_rotation_quaternion();    
}

Vector3 CharacterModel::get_target_facing_direction() const {
    return target_facing_direction.xform(Vector3(0.0, 0.0, -1.0f));
}
