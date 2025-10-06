#include "terrain_heightmap_combine_layer.h"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "profiling.h"
#include "segment_quadtree.h"
#include "terrain_generator/terrain_heightmap.h"
#include "terrain_generator/terrain_roads.h"
#include "tracy/Tracy.hpp"
#include <limits>

int TerrainFinalCombineLayer::get_chunk_size() const {
    return 2048;
}

Ref<ChunkinatorChunk> TerrainFinalCombineLayer::instantiate_chunk() {
    Ref<TerrainFinalCombineChunk> chunk;
    chunk.instantiate();
    chunk->layer = this;
    return chunk;
}

float TerrainFinalCombineLayer::sample_height(const Vector2 &p_world_position) const {
    FuncProfile;
    float height = 0.0f;
    
    float dist_to_road = 0.0f;
    Vector2 closest_point;
    SegmentQuadtree::QuadTreeSegment segment;    
    
    const bool has_closest_segment = get_road_connection_layer()->get_distance_to_closest_road_segment(p_world_position, segment, &dist_to_road, &closest_point);

    const float multiplier = 1000.0f;

    if (has_closest_segment && dist_to_road < 250.0f) {
        height = heightmap_layer->sample_height(closest_point) * multiplier;
        if (dist_to_road > 150.0f) {
            float our_height = heightmap_layer->sample_height(p_world_position) * multiplier;
            float blend_factor = Math::inverse_lerp(150.0f, 250.0f, dist_to_road);
            height = Math::lerp(height, our_height, blend_factor * blend_factor);
        }
    } else {
        height = heightmap_layer->sample_height(p_world_position) * multiplier;
    }
    
    return height;
}

Ref<TerrainHeightmapLayer> TerrainFinalCombineLayer::get_heightmap_layer() const {
    if (!heightmap_layer.is_valid()) {
        const_cast<TerrainFinalCombineLayer*>(this)->heightmap_layer = get_layer("Heightmap Layer");
        DEV_ASSERT(heightmap_layer.is_valid());
    }
    return heightmap_layer;
}

Ref<TerrainRoadConnectionLayer> TerrainFinalCombineLayer::get_road_connection_layer() const {
    if (!road_connection_layer.is_valid()) {
        const_cast<TerrainFinalCombineLayer*>(this)->road_connection_layer = get_layer("Road Connections");
        DEV_ASSERT(road_connection_layer.is_valid());
    }
    return road_connection_layer;
}

void TerrainFinalCombineChunk::generate() {
    FuncProfile;
    const Rect2 rect = get_chunk_bounds();
    Ref<TerrainRoadConnectionLayer> road_connection_layer = layer->get_road_connection_layer();
    
    road_sdf = Image::create_empty(road_sdf_size, road_sdf_size, false, Image::FORMAT_RF);
    for (int x = 0; x < road_sdf_size; x++) {
        double sample_x = rect.position.x + rect.size.x * (x / (float)(road_sdf_size-1));
        for (int y = 0; y < road_sdf_size; y++) {
            const Vector2 point_to_sample = Vector2(sample_x, rect.position.y + rect.size.y * (y / (float)(road_sdf_size-1)));
            float distance = 0.0f;
            SegmentQuadtree::QuadTreeSegment segment;
            
            if (!road_connection_layer->get_distance_to_closest_road_segment(point_to_sample, segment, &distance)) {
                distance = std::numeric_limits<float>::max();       
            }

            road_sdf->set_pixel(x, y, Color(distance, 0.0, 0.0));
        }
    }

    height_map.instantiate();
    height_map = Image::create_empty(heightmap_size, heightmap_size, false, Image::FORMAT_RF);
    Ref<TerrainHeightmapLayer> heightmap_layer = layer->get_heightmap_layer();
    for (int x = 0; x < heightmap_size; x++) {
        double sample_x = rect.position.x + rect.size.x * (x / (float)(heightmap_size-1));
        for (int y = 0; y < heightmap_size; y++) {
            const Vector2 point_to_sample = Vector2(sample_x, rect.position.y + rect.size.y * (y / (float)(heightmap_size-1)));
            double height = layer->sample_height(point_to_sample);
            
            height_map->set_pixel(x, y, Color(height, 0.0, 0.0, 0.0));
        }
    }
}

void TerrainFinalCombineChunk::debug_draw(ChunkinatorDebugDrawer *p_debug_drawer) const {
    if (!debug_draw_texture.is_valid()) {
        const_cast<TerrainFinalCombineChunk*>(this)->debug_draw_texture = ImageTexture::create_from_image(height_map);
    }

    if (!debug_road_sdf_texture.is_valid()) {
        Ref<Image> road_sdf_debug_img = Image::create_empty(road_sdf_size, road_sdf_size, false, Image::FORMAT_RGBA8);
        Rect2 rect = get_chunk_bounds();
        for (int x = 0; x < road_sdf_size; x++) {
            for (int y = 0; y < road_sdf_size; y++) {
                float dist = road_sdf->get_pixel(x, y).r;

                float alpha = 1.0f;

                if (dist > 300.0f) {
                    alpha = 0.0f;
                } 

                dist = dist / 300.0f;
                road_sdf_debug_img->set_pixel(x, y, Color(dist, dist, dist, alpha));
            }
        }
        const_cast<TerrainFinalCombineChunk*>(this)->debug_road_sdf_texture = ImageTexture::create_from_image(road_sdf_debug_img);
    }

    p_debug_drawer->draw_texture(debug_draw_texture, get_chunk_bounds());
    p_debug_drawer->draw_texture(debug_road_sdf_texture, get_chunk_bounds());

}

Ref<Image> TerrainFinalCombineChunk::get_height_map() const {
    return height_map;
}

Ref<Image> TerrainFinalCombineChunk::get_road_sdf() const {
    return road_sdf;
}
