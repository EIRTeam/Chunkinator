#pragma once

#include "godot_cpp/classes/node3d.hpp"
#include "bind_macros.h"

using namespace godot;

class WeaponModel : public Node3D {
    GDCLASS(WeaponModel, Node3D);
    Node3D *muzzle_location = nullptr;
    static void _bind_methods();
public:
    MAKE_SETTER_GETTER_VALUE(Node3D*, muzzle_location, muzzle_location);
};