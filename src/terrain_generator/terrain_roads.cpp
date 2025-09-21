#include "terrain_roads.h"
#include "godot_cpp/core/math.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/templates/local_vector.hpp"
#include "terrain_generator/random_point_scatter.h"

int TerrainRoadConnectionLayer::get_chunk_size() const {
    return 16384;
}

Ref<ChunkinatorChunk> TerrainRoadConnectionLayer::instantiate_chunk() {
    Ref<TerrainRoadConnectionChunk> chunk;
    chunk.instantiate();
    chunk->layer = this;
    return chunk;
}

LocalVector<Vector2> TerrainRoadConnectionLayer::get_points_in_radius(Vector2 p_position, float p_radius) const {
    Ref<RandomPointLayer> layer = get_layer("Road Random Points");
    LocalVector<Vector2> points = layer->get_points_in_bounds(Rect2(p_position - Vector2(p_radius, p_radius), Vector2(p_radius, p_radius)));
    const float radius_squared = p_radius * p_radius;
    for (int i = points.size()-1; i >= 0; i--) {
        if (points[i].distance_squared_to(p_position) > p_radius) {
            points.remove_at(i);
        }
    }
    return points;
}

LocalVector<Vector2> TerrainRoadConnectionLayer::get_points_in_bounds(Rect2 p_bounds) const {
    Ref<RandomPointLayer> layer = get_layer("Road Random Points");
    return layer->get_points_in_bounds(p_bounds);
}


struct PointSorter {
    struct PointDistance {
        int idx;
        float distance;
    };
    _FORCE_INLINE_ bool operator()(const PointDistance &l, const PointDistance &r) const {
        return l.distance < r.distance;
    }
};

void sort_by_dist_to_point(Vector2 p_point, LocalVector<Vector2> &ri_points) {
    LocalVector<PointSorter::PointDistance> distances_squared;
    distances_squared.reserve(ri_points.size());

    for (int i = 0; i < ri_points.size(); i++) {
        distances_squared.push_back({
            .idx = i,
            .distance = ri_points[i].distance_squared_to(p_point)
        });
    }

    distances_squared.sort_custom<PointSorter>();
    LocalVector<Vector2> positions;
    positions.resize(ri_points.size());

    for (int i = 0; i < ri_points.size(); i++) {
        positions[i] = ri_points[distances_squared[i].idx];
    }

    ri_points = positions;
}

void TerrainRoadConnectionChunk::generate() {
    Rect2 check_rect = get_chunk_bounds().grow(connection_radius);
    LocalVector<Vector2> points = layer->get_points_in_bounds(check_rect);
    const float check_dist_squared = connection_radius * connection_radius;

    for (int i = 0; i < points.size(); i++) {
        LocalVector<Vector2> points_in_radius = layer->get_points_in_radius(points[i], connection_radius);
        sort_by_dist_to_point(points[i], points_in_radius);
        int added_points = 0;
        for (int j = 0; j < points.size(); j++) {
            const float dist_sq = points[i].distance_squared_to(points[j]);
            if (Math::is_zero_approx(dist_sq)) {
                continue;
            }
            if (dist_sq < check_dist_squared) {
                added_points++;
                connections.push_back({
                    .from = points[i],
                    .to = points[j]
                });
            }
            if (added_points > maximum_connections) {
                break;
            }
        }
    }
}

void TerrainRoadConnectionChunk::debug_draw(ChunkinatorDebugDrawer *p_debug_drawer) const {
    for (const Connection &connection : connections) {
        if (get_chunk_bounds().has_point((connection.from + connection.to) * 0.5)) {
            p_debug_drawer->draw_line(connection.from, connection.to);
        }
    }
    Rect2 check_rect = get_chunk_bounds().grow(connection_radius);
};