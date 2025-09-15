#include "chunkinator.h"
#include "chunkinator/chunkinator_layer.h"
#include "godot_cpp/core/error_macros.hpp"
#include <optional>
#include <queue>

Ref<ChunkinatorLayer> Chunkinator::get_layer(StringName p_name) const {
    for (auto it : layers) {
        if (it->name == p_name) {
            return it;
        }
    }
    return nullptr;
}

LocalVector<Ref<ChunkinatorLayer>> Chunkinator::get_leaves() {
    LocalVector<Ref<ChunkinatorLayer>> leaves;
    for (Ref<ChunkinatorLayer> layer : layers) {
        if (layer->children.is_empty()) {
            leaves.push_back(layer);
        }
    }
    return leaves;
}

Vector2i get_chunk_indices(Vector2i p_pos, int p_chunk_size) {
    Vector2i chunk = Vector2i (
        p_pos.x / p_chunk_size,
        p_pos.y / p_chunk_size
    );
    
    // Handle negative positions
    if (p_pos.x < 0 && p_pos.x % p_chunk_size != 0) chunk.x--;
    if (p_pos.y < 0 && p_pos.y % p_chunk_size != 0) chunk.x--;

    return chunk;
}

Rect2i Chunkinator::_snap_with_padding(Rect2i p_rect, int p_chunk_size, Vector2i p_padding) const {
    const Vector2i min_chunk_without_padding = get_chunk_indices(p_rect.position, p_chunk_size);
    const Vector2i max_chunk_without_padding = get_chunk_indices(p_rect.position, p_chunk_size);
    const Vector2i min_chunk = get_chunk_indices(min_chunk_without_padding * p_chunk_size - p_padding, p_chunk_size);
    const Vector2i max_chunk = get_chunk_indices(max_chunk_without_padding * p_chunk_size - p_padding, p_chunk_size);

    return Rect2i(min_chunk, max_chunk - min_chunk);
}

std::optional<Chunkinator::LayerDependency> Chunkinator::find_layer_dependency(StringName p_parent_name, StringName p_child_name) const {
    for (const LayerDependency &dep : layer_dependencies) {
        if (dep.parent == p_parent_name && dep.child == p_child_name) {
            return dep;
        }
    }
    return std::nullopt;
}

void Chunkinator::_recalculate_bounds(Ref<ChunkinatorLayer> p_node, Rect2i p_child_bounds, Vector2i p_child_padding) {

    // First calculate our own rect snapped to chunk boundaries
    const Rect2i own_rect = _snap_with_padding(p_child_bounds, p_node->get_chunk_size(), p_child_padding);

    p_node->rect_to_generate = own_rect;

    for (StringName parent : p_node->parents) {
        std::optional<LayerDependency> layer_dep = find_layer_dependency(parent, p_node->name);
        
        ERR_FAIL_COND_MSG(!layer_dep.has_value(), "Bug?");

        Ref<ChunkinatorLayer> parent_node = get_layer(parent);

        ERR_FAIL_COND_MSG(!parent_node.is_valid(), "Bug?");

        _recalculate_bounds(parent_node, own_rect, layer_dep->padding);
    }
}

void Chunkinator::recalculate_bounds() {
    LocalVector<Ref<ChunkinatorLayer>> leaves = get_leaves();

    // TODO: Add generation rect
    const Rect2i generation_rect(0, 0, 100, 100);

    for (Ref<ChunkinatorLayer> leaf : leaves) {
        _recalculate_bounds(leaf, generation_rect, Vector2i());

    }
}

void Chunkinator::add_layer_dependency(StringName p_parent_name, StringName p_child_name, Vector2i p_padding) {
    Ref<ChunkinatorLayer> parent_layer = get_layer(p_parent_name);
    Ref<ChunkinatorLayer> child_layer = get_layer(p_child_name);
    ERR_FAIL_COND_MSG(parent_layer == nullptr, vformat("Parent layer with name %s not found!", p_parent_name));
    ERR_FAIL_COND_MSG(child_layer == nullptr, vformat("Child layer with name %s not found!", p_child_name));

    layer_dependencies.push_back({
        .padding = p_padding,
        .parent = p_parent_name,
        .child = p_child_name,
    });

    child_layer->parents.push_back(p_parent_name);
    parent_layer->children.push_back(p_parent_name);
}

void Chunkinator::build() {
    LocalVector<int> in_degrees;

    in_degrees.reserve(layers.size());

    std::queue<int> processing_nodes;
    for (int i = 0; i < layers.size(); i++) {
        in_degrees.push_back(layers[i]->parents.size());

        if (layers[i]->parents.size() == 0) {
            processing_nodes.push(i);
        }
    }

    // Calculate DAG levels
    int processed_nodes = 0;
    for (; !processing_nodes.empty(); processing_nodes.pop()) {
        processed_nodes++;
        Ref<ChunkinatorLayer> current_layer = layers[processing_nodes.front()];
        for (StringName child : current_layer->children) {
            Ref<ChunkinatorLayer> child_layer = get_layer(child);
            int child_idx = layers.find(child_layer);
            child_layer->dag_level = Math::max(child_layer->dag_level, current_layer->dag_level+1);
            in_degrees[child_idx]--;
            if (in_degrees[child_idx] == 0) {
                processing_nodes.push(child_idx);
            }
        }
    }

    ERR_FAIL_COND_MSG(processed_nodes < layers.size(), "Graph contains a cycle!");
}
