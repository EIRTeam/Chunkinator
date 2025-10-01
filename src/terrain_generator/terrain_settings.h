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

    void set_height_noise_layers_bind(TypedArray<TerrainHeightNoiseLayerSettings> p_noise_layers);
    TypedArray<TerrainHeightNoiseLayerSettings> get_height_noise_layers_bind() const;

public:
    void set_terrain_material(Ref<ShaderMaterial> p_material);
    Ref<ShaderMaterial> get_terrain_material() const;
    int get_height_layer_count() const;
    Ref<TerrainHeightNoiseLayerSettings> get_height_layer(int p_idx) const;

    static void _bind_methods();
};