#include "superchunk_debugger.h"
#include "godot_cpp/classes/font.hpp"

void SuperchunkDebugger::_bind_methods() {
}

void SuperchunkDebugger::set_worldman(WorldMan *p_worldman) {
    wm = p_worldman;
}

void SuperchunkDebugger::set_superchunk(Vector2i p_superchunk) {
    superchunk = p_superchunk;
    queue_redraw();
}

void SuperchunkDebugger::_draw() {
    const WorldMan::WorldManSuperchunk *sc = wm->find_superchunk(superchunk);
    if (sc == nullptr) {
        return;
    }

    if (sc->state != WorldMan::WorldManSuperchunk::State::IDLE) {
        return;
    }

    LocalVector<QuadTree::LeafInformation> leaves;
    sc->quadtree->get_leafs(leaves);
    
    const int min_side = get_size()[get_size().min_axis_index()];
    const int quadtree_side_size = 4096;
    const float scaling_factor = (((float)min_side) / quadtree_side_size);
    const Vector2 center = get_size() * 0.5f;

    const Vector2 center_offset = center - (Vector2(min_side, min_side) * 0.5);

    auto remap_chunk_local_pos_to_display = [scaling_factor, center_offset, min_side](Vector2 p_position) {
        return Vector2(p_position * scaling_factor + center_offset);
    };

    auto remap_chunk_rect_to_display = [scaling_factor, center_offset, min_side, remap_chunk_local_pos_to_display](Rect2 p_rect) {
        return Rect2(remap_chunk_local_pos_to_display(p_rect.position), p_rect.size * scaling_factor);
    };

    for (const QuadTree::LeafInformation &chunk : leaves) {
        const Rect2 rect = remap_chunk_rect_to_display(Rect2(chunk.rect));
        draw_rect(rect, Color(1.0f, 0.0f, 0.0f, 1.0f), false);
        draw_string(get_theme_default_font(), rect.get_center(), vformat("%d", chunk.depth));
    }

    // Draw camera
    draw_circle(remap_chunk_local_pos_to_display(wm->camera_position - wm->get_superchunk_world_rect(sc->position).position) , 10.0, Color(1.0, 1.0, 0.0));
}
