#pragma once

#include "game/movement_shared.h"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/physics_test_motion_result3d.hpp"
#include "godot_cpp/classes/shape3d.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include "movement_settings.h"

class BaseMovement {
public:
    enum MovementPass {
        LATERAL,
        GRAVITY,
        SNAP
    };
private:
    struct MovementIterParams {
        const Transform3D &starting_trf;
        const Vector3 &motion;
        const Vector3 &desired_movement_direction;
        const MovementPass movement_pass;
        const float motion_margin = 0.001f;
    };

    struct MovementIterResult {
        bool done = false;
        bool hit_something = false;
        bool movement_snapped = false;
        Vector3 remaining_velocity;
        Transform3D new_transform;
        Ref<PhysicsTestMotionResult3D> kinematic_collision_result;
    };

    struct MovementPassResult {
        bool hit_something = false;
        bool movement_snapped = false;
    };

    Ref<MovementSettings> movement_settings;
    Node3D *owner_node = nullptr;

    Ref<Shape3D> body_shapes_per_stance[Movement::STANCE_MAX];

    RID body;

    struct StairSnapResult {
        bool snapped = false;
        Vector3 resulting_position;
    };

    float vertical_velocity = 0.0f;
    Vector3 desired_velocity;

    void _movement_iter(const MovementIterParams &p_params, MovementIterResult &r_out) const;
    bool _test_move(const Transform3D &p_trf, const Vector3 &p_motion, Ref<PhysicsTestMotionResult3D> &r_result, float p_margin = 0.001f) const;
    void _try_snap_up_stair(const Vector3 &p_movement_dir, const Transform3D &p_starting_trf, StairSnapResult &r_result) const;
    void _do_movement_pass(const MovementPass p_pass, float p_delta, MovementPassResult &r_out);
public:
    void set_desired_velocity(const Vector3 &p_desired_vel);
    Vector3 get_desired_velocity() const;
    void initialize(Ref<MovementSettings> p_movement_settings, Node3D *p_owner);
    void update(float p_delta);
    void _handle_collision(const Vector3 &p_desired_velocity, const Ref<PhysicsTestMotionResult3D> &p_collision_result)
};