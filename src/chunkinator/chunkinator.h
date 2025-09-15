#pragma once

#include "chunkinator/chunkinator_layer.h"

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/templates/hash_map.hpp"
#include "godot_cpp/templates/local_vector.hpp"
#include "godot_cpp/variant/string_name.hpp"
#include <optional>

using namespace godot;

class Chunkinator {

    struct LayerDependency {
        Vector2i padding;
        StringName parent;
        StringName child;
    };

    LocalVector<LayerDependency> layer_dependencies;

    LocalVector<Ref<ChunkinatorLayer>> layers;

    Ref<ChunkinatorLayer> get_layer(StringName p_name) const;

    LocalVector<Ref<ChunkinatorLayer>> get_leaves();
    
    Rect2i _snap_with_padding(Rect2i p_rect, int p_chunk_size, Vector2i p_padding) const;
    
    std::optional<LayerDependency> find_layer_dependency(StringName p_parent_name, StringName p_child_name) const;
    void _recalculate_bounds(Ref<ChunkinatorLayer> p_node, Rect2i p_child_bounds, Vector2i p_child_padding);
    void recalculate_bounds();
    void insert_layer(StringName p_layer_name, Ref<ChunkinatorLayer> p_layer);
    void add_layer_dependency(StringName p_parent_name, StringName p_child_name, Vector2i p_padding);
    void build();
};