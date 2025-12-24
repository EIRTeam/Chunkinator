#pragma once

#include "game/movement_shared.h"
#include "godot_cpp/classes/resource.hpp"
#include "bind_macros.h"

using namespace godot;

class MovementSettings : public Resource {
    GDCLASS(MovementSettings, Resource);

    float radius = 0.3f;
    float acceleration[Movement::MovementStance::STANCE_MAX] = {100.0f, 100.0f};
    float max_speed[Movement::MovementStance::STANCE_MAX] = { 3.0f, 1.5f };
    float height[Movement::MovementStance::STANCE_MAX] = { 1.5f, 0.8f };

    float max_slope_angle = Math::deg_to_rad(60.0f);
    float max_step_height = 0.3f;
    float min_step_depth = 0.1f;
    float snap_to_ground_height = 0.6f;
    float terminal_velocity = 54.0f;
    float gravity = 10.0f;
public:
    static void _bind_methods();

    MAKE_SETTER_GETTER_FLOAT_VALUE(crouching_acceleration, acceleration[Movement::MovementStance::CROUCHING]);
    MAKE_SETTER_GETTER_FLOAT_VALUE(standing_acceleration, acceleration[Movement::MovementStance::STANDING]);
    MAKE_SETTER_GETTER_FLOAT_VALUE(crouching_max_speed, max_speed[Movement::MovementStance::CROUCHING]);
    MAKE_SETTER_GETTER_FLOAT_VALUE(standing_max_speed, max_speed[Movement::MovementStance::STANDING]);
    MAKE_SETTER_GETTER_FLOAT_VALUE(crouching_height, height[Movement::MovementStance::CROUCHING]);
    MAKE_SETTER_GETTER_FLOAT_VALUE(standing_height, height[Movement::MovementStance::STANDING]);
    MAKE_SETTER_GETTER_FLOAT_VALUE(radius, radius);
    MAKE_SETTER_GETTER_FLOAT_VALUE(max_slope_angle, max_slope_angle);
    MAKE_SETTER_GETTER_FLOAT_VALUE(max_step_height, max_step_height);
    MAKE_SETTER_GETTER_FLOAT_VALUE(min_step_depth, min_step_depth);
    MAKE_SETTER_GETTER_FLOAT_VALUE(snap_to_ground_height, snap_to_ground_height);
    MAKE_SETTER_GETTER_FLOAT_VALUE(terminal_velocity, terminal_velocity);
    MAKE_SETTER_GETTER_FLOAT_VALUE(gravity, gravity);

    float get_stance_height(int p_stance) const;
};