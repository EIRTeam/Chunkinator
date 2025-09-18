#include "chunkinator_layer.h"
#include "chunkinator/chunkinator_chunk.h"

Ref<ChunkinatorChunk> ChunkinatorLayer::get_chunk_by_index(Vector2i p_chunk_idx) const {
    for (Ref<ChunkinatorChunk> chunk : chunks) {
        if (chunk->get_chunk_index() == p_chunk_idx) {
            return chunk;
        }
    }

    return nullptr;
}

