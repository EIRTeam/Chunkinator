#pragma once

#include "godot_cpp/classes/curve.hpp"
#include "godot_cpp/classes/fast_noise_lite.hpp"
#include "godot_cpp/classes/packed_scene.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/shader_material.hpp"
#include "godot_cpp/templates/local_vector.hpp"

using namespace godot;

class TerrainHeightNoiseLayerSettings : public Resource {
    GDCLASS(TerrainHeightNoiseLayerSettings, Resource);
    Ref<FastNoiseLite> noise;
    Ref<Curve> curve;

public:
    Ref<Curve> get_curve() const { return curve; }
    void set_curve(const Ref<Curve> &p_curve) { curve = p_curve; }

    Ref<FastNoiseLite> get_noise() const { return noise; }
    void set_noise(const Ref<FastNoiseLite> &p_noise) { noise = p_noise; }

    static void _bind_methods();
};

class TerrainScattererElementSettings : public Resource {
    GDCLASS(TerrainScattererElementSettings, Resource);

    Ref<PackedScene> scene;
    float probability = 1.0f;
    float max_angle_radians = Math::deg_to_rad(180.f);

public:
    float get_probability() const { return probability; }
    void set_probability(float p_probability) { probability = p_probability; }

    float get_max_angle_radians() const { return max_angle_radians; }
    void set_max_angle_radians(float p_max_angle_radians) { max_angle_radians = p_max_angle_radians; }

    Ref<PackedScene> get_scene() const { return scene; }
    void set_scene(const Ref<PackedScene> &p_scene) { scene = p_scene; }

    static void _bind_methods();
};

class TerrainScatterLayerSettings : public Resource {
    GDCLASS(TerrainScatterLayerSettings, Resource);
    StringName debug_name;
    int layer_chunk_size = 128;
    int layer_element_count_per_side = 16;
    float none_probability = 0.0f;
    Vector<Ref<TerrainScattererElementSettings>> elements;

public:
    float get_none_probability() const { return none_probability; }
    void set_none_probability(float p_none_probability) { none_probability = p_none_probability; }

    int get_layer_chunk_size() const { return layer_chunk_size; }
    void set_layer_chunk_size(int p_layer_chunk_size) { layer_chunk_size = p_layer_chunk_size; }

    int get_layer_element_count_per_side() const { return layer_element_count_per_side; }
    void set_layer_element_count_per_side(int p_layer_element_count_per_side) { layer_element_count_per_side = p_layer_element_count_per_side; }

    Vector<Ref<TerrainScattererElementSettings>> get_elements() const { return elements; }
    void set_elements(const Vector<Ref<TerrainScattererElementSettings>> &p_elements) { elements = p_elements; }
    
    TypedArray<TerrainScattererElementSettings> get_elements_bind() const;
    void set_elements_bind(const TypedArray<TerrainScattererElementSettings> &p_elements);

    static void _bind_methods();

    StringName get_debug_name() const { return debug_name; }
    void set_debug_name(const StringName &debug_name_) { debug_name = debug_name_; }
};

class TerrainSettings : public Resource {
    GDCLASS(TerrainSettings, Resource);

    Vector<Ref<TerrainHeightNoiseLayerSettings>> height_noise_layers;
    Ref<ShaderMaterial> terrain_material;
    
    int terrain_base_heightmap_chunk_size = 4096;
    int terrain_base_heightmap_size = 128;

    int geometry_chunk_size = 2048;
    int geometry_chunk_heightmap_size = 128;
    int geometry_chunk_heightmap_texture_array_layer_count = 128;
    int geometry_chunks_spatial_map_size = 128;
    int mesh_quality = 17;
    float lod_radius_threshold_multiplier = 1.0f;

    Vector<Ref<TerrainScatterLayerSettings>> scatter_layers;

    void set_height_noise_layers_bind(TypedArray<TerrainHeightNoiseLayerSettings> p_noise_layers);
    TypedArray<TerrainHeightNoiseLayerSettings> get_height_noise_layers_bind() const;
    
    void set_scatter_layers_bind(TypedArray<TerrainScatterLayerSettings> p_scatter_layers);
    TypedArray<TerrainScatterLayerSettings> get_scatter_layers_bind() const;
public:
    void set_terrain_material(Ref<ShaderMaterial> p_material);
    Ref<ShaderMaterial> get_terrain_material() const;
    int get_height_layer_count() const;
    Ref<TerrainHeightNoiseLayerSettings> get_height_layer(int p_idx) const;

    static void _bind_methods();

    int get_geometry_chunk_heightmap_size() const { return geometry_chunk_heightmap_size; }
    void set_geometry_chunk_heightmap_size(int geometry_chunk_heightmap_size_) { geometry_chunk_heightmap_size = geometry_chunk_heightmap_size_; }

    int get_geometry_chunk_heightmap_texture_array_layer_count() const { return geometry_chunk_heightmap_texture_array_layer_count; }
    void get_geometry_chunk_heightmap_texture_array_layer_count(int p_geometry_chunk_heightmap_texture_array_layer_count) { geometry_chunk_heightmap_texture_array_layer_count = p_geometry_chunk_heightmap_texture_array_layer_count; }

    int get_geometry_chunks_spatial_map_size() const { return geometry_chunks_spatial_map_size; }
    void set_geometry_chunks_spatial_map_size(int p_geometry_chunks_spatial_map_size) { geometry_chunks_spatial_map_size = p_geometry_chunks_spatial_map_size; }

    int get_geometry_chunk_size() const { return geometry_chunk_size; }
    void set_geometry_chunk_size(int p_geometry_chunk_size) { geometry_chunk_size = p_geometry_chunk_size; }

    float get_lod_radius_threshold_multiplier() const { return lod_radius_threshold_multiplier; }
    void set_lod_radius_threshold_multiplier(float p_lod_radius_threshold_multiplier) { lod_radius_threshold_multiplier = p_lod_radius_threshold_multiplier; }

    int get_mesh_quality() const { return mesh_quality; }
    void set_mesh_quality(int p_mesh_quality) { mesh_quality = p_mesh_quality; }

    int get_scatter_layer_count() const;
    Ref<TerrainScatterLayerSettings> get_scatter_layer(int p_idx) const;
};