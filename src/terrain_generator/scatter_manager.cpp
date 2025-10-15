#include "scatter_manager.h"
#include "godot_cpp/core/error_macros.hpp"
#include "terrain_generator/scatter.h"

void ScatterManager::spawn_chunk(const Vector2i &p_chunk) {
    Ref<ScatterLayer> scatter_layer = get_layer();
    Ref<ScatterChunk> chunk = scatter_layer->get_chunk_by_index(p_chunk);
    DEV_ASSERT(chunk.is_valid());

    const LocalVector<ScatterChunk::ScatterElement> &elements = chunk->get_elements();

    Node3D *root = memnew(Node3D);

    for (const ScatterChunk::ScatterElement &element : elements) {
        Node *scene_instance = element.scene->instantiate();
        Node3D *scene_instance_node_3d = Object::cast_to<Node3D>(scene_instance);
        if (scene_instance_node_3d == nullptr) {
            ERR_FAIL_MSG(vformat("Scatter scene %s was not of type Node3D!", element.scene->get_path()));
            memdelete(root);
            memdelete(scene_instance);
            return;
        }

        root->add_child(scene_instance_node_3d);
        scene_instance_node_3d->set_transform(element.transform);
    }
    add_child(root);

    scatter_chunks.insert(p_chunk, root);
}

void ScatterManager::unload_chunk(const Vector2i &p_chunk) {
    DEV_ASSERT(scatter_chunks.has(p_chunk));
    memdelete(scatter_chunks[p_chunk]);
    scatter_chunks.erase(p_chunk);
}
