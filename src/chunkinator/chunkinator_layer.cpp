#include "chunkinator_layer.h"
#include "chunkinator/chunkinator_chunk.h"
#include "chunkinator/chunkinator.h"
#include "godot_cpp/core/error_macros.hpp"

Ref<ChunkinatorChunk> ChunkinatorLayer::get_chunk_by_index(Vector2i p_chunk_idx) const {
    for (Ref<ChunkinatorChunk> chunk : chunks) {
        if (chunk->get_chunk_index() == p_chunk_idx) {
            return chunk;
        }
    }

    return nullptr;
}

Ref<ChunkinatorChunk> ChunkinatorLayer::get_chunk(int p_idx) const {
    ERR_FAIL_INDEX_V(p_idx, chunks.size(), nullptr);
    return chunks[p_idx];
}

Ref<ChunkinatorChunk> ChunkinatorLayer::get_chunk_in_position(const Vector2 &p_position) const {
    Vector2i chunk_idx = (p_position / get_chunk_size()).floor();
    Ref<ChunkinatorChunk> chunk = get_chunk_by_index(chunk_idx);
    return chunk;
}

int ChunkinatorLayer::get_chunk_count() const {
    return chunks.size();
}

StringName ChunkinatorLayer::get_name() const {
    return name;
}

Ref<ChunkinatorLayer> ChunkinatorLayer::get_layer(const StringName p_layer_name) const {
    return chunkinator->get_layer(p_layer_name);
}

