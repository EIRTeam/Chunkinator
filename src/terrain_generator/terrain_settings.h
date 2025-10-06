#pragma once

#include "godot_cpp/classes/curve.hpp"
#include "godot_cpp/classes/fast_noise_lite.hpp"
#include "godot_cpp/classes/packed_scene.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/shader_material.hpp"

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

class TerrainScattererElement : public Resource {
    GDCLASS(TerrainScattererElement, Resource);

    Ref<PackedScene> scene;
    int amount_per_scatterer_chunk = 10;
    float probability = 1.0f;
};

class TerrainSettings : public Resource {
    GDCLASS(TerrainSettings, Resource);

    Vector<Ref<TerrainHeightNoiseLayerSettings>> height_noise_layers;
    Ref<ShaderMaterial> terrain_material;
    
    int terrain_base_heightmap_chunk_size = 4096;
    int terrain_base_heightmap_size = 128;

    int geometry_chunk_size = 2048;
    int geometry_chunk_heightmap_size = 64;
    int geometry_chunk_heightmap_texture_array_layer_count = 128;
    int geometry_chunks_spatial_map_size = 128;

    void set_height_noise_layers_bind(TypedArray<TerrainHeightNoiseLayerSettings> p_noise_layers);
    TypedArray<TerrainHeightNoiseLayerSettings> get_height_noise_layers_bind() const;

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
    void set_geometry_chunks_spatial_map_size(int geometry_chunks_spatial_map_size_) { geometry_chunks_spatial_map_size = geometry_chunks_spatial_map_size_; }
};