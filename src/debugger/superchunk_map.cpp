#include "superchunk_map.h"

#include "godot_cpp/classes/font.hpp"
#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/classes/input_event_mouse_button.hpp"

void SuperchunkMap::set_worldman(WorldMan *p_worldman) {
    wm = p_worldman;
}

void SuperchunkMap::_gui_input(const Ref<InputEvent> &p_event) {
    const Ref<InputEventMouseButton> ev = p_event;

    if (ev.is_valid() && ev->is_pressed() && !ev->is_echo()) {
        const Vector2 mouse_local = get_local_mouse_position();
        for (const SuperchunkDrawData &sc : superchunk_draw_datas) {
            if (sc.local_rect.has_point(mouse_local)) {
                emit_signal("superchunk_selected", sc.superchunk_grid_position);
                accept_event(); 
                break;
            }
        }

    }
}

SuperchunkMap::SuperchunkMap() {
    set_clip_contents(true);
}

void SuperchunkMap::_bind_methods() {
    ADD_SIGNAL(MethodInfo("superchunk_selected", PropertyInfo(Variant::VECTOR2I, "superchunk")));
}

void SuperchunkMap::set_draw_range(float p_draw_range) {
    draw_range = p_draw_range;
    queue_redraw();
}

void SuperchunkMap::_draw() {
	const int window_shortest_axis = get_size()[get_size().min_axis_index()];
    const float scaling_factor = (window_shortest_axis / draw_range) * 0.5; 
    const Vector2 camera_position = wm->camera_position;
    
    const auto remap_world_position_to_display = [this, camera_position, scaling_factor, window_shortest_axis](Vector2 p_world_position) {
        // Move world origin to the camera position
        const Vector2 centered_pos = p_world_position - camera_position;
        return centered_pos * scaling_factor + Vector2(get_size()) * 0.5;
    };

    const auto remap_world_rect_to_display = [remap_world_position_to_display, scaling_factor](Rect2 p_world_rect) {
        return Rect2(remap_world_position_to_display(p_world_rect.position), p_world_rect.size * scaling_factor);
    };

    superchunk_draw_datas.clear();
    superchunk_draw_datas.reserve(wm->superchunks.size());

    for (const WorldMan::WorldManSuperchunk &sc : wm->superchunks) {
        SuperchunkDrawData draw_data = {
            .superchunk_grid_position = sc.position,
            .local_rect = remap_world_rect_to_display(wm->get_superchunk_world_rect(sc.position))
        };
        superchunk_draw_datas.push_back(draw_data);

        draw_rect(draw_data.local_rect, Color(1.0, 0.0, 0.0, 1.0));
        draw_string(get_theme_default_font(), draw_data.local_rect.get_center(), vformat("%d, %d", draw_data.superchunk_grid_position.x, draw_data.superchunk_grid_position.y));
    }
}