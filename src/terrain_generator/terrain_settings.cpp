#include "terrain_settings.h"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/error_macros.hpp"

void TerrainSettings::set_height_noise_layers_bind(TypedArray<TerrainHeightNoiseLayerSettings> p_noise_layers) {
    height_noise_layers.clear();
    height_noise_layers.resize(p_noise_layers.size());

    Ref<TerrainHeightNoiseLayerSettings> *height_noise_layers_w = height_noise_layers.ptrw();

    for (int i = 0; i < p_noise_layers.size(); i++) {
        height_noise_layers_w[i] = p_noise_layers[i];
    }
}

void TerrainSettings::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_height_noise_layers", "curve"), &TerrainSettings::set_height_noise_layers_bind);
    ClassDB::bind_method(D_METHOD("get_height_noise_layers"), &TerrainSettings::get_height_noise_layers_bind);
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "height_noise_layers", PROPERTY_HINT_ARRAY_TYPE, "TerrainHeightNoiseLayerSettings"), "set_height_noise_layers", "get_height_noise_layers");
    
    ClassDB::bind_method(D_METHOD("set_terrain_material", "terrain_material"), &TerrainSettings::set_terrain_material);
    ClassDB::bind_method(D_METHOD("get_terrain_material"), &TerrainSettings::get_terrain_material);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "terrain_material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"), "set_terrain_material", "get_terrain_material");
}

TypedArray<TerrainHeightNoiseLayerSettings> TerrainSettings::get_height_noise_layers_bind() const {
    TypedArray<TerrainHeightNoiseLayerSettings> out;
    out.resize(height_noise_layers.size());

    for (int i = 0; i < height_noise_layers.size(); i++) {
        out[i] = height_noise_layers[i];
    }

    return out;
}

void TerrainSettings::set_terrain_material(Ref<ShaderMaterial> p_material) {
    terrain_material = p_material;
}

Ref<ShaderMaterial> TerrainSettings::get_terrain_material() const {
    return terrain_material;
}

int TerrainSettings::get_height_layer_count() const {
    return height_noise_layers.size();
}

Ref<TerrainHeightNoiseLayerSettings> TerrainSettings::get_height_layer(int p_idx) const {
    ERR_FAIL_INDEX_V(p_idx, height_noise_layers.size(), nullptr);
    return height_noise_layers[p_idx];
}

void TerrainHeightNoiseLayerSettings::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_curve", "curve"), &TerrainHeightNoiseLayerSettings::set_curve);
    ClassDB::bind_method(D_METHOD("get_curve"), &TerrainHeightNoiseLayerSettings::get_curve);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_curve", "get_curve");

    ClassDB::bind_method(D_METHOD("set_noise", "noise"), &TerrainHeightNoiseLayerSettings::set_noise);
    ClassDB::bind_method(D_METHOD("get_noise"), &TerrainHeightNoiseLayerSettings::get_noise);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "noise", PROPERTY_HINT_RESOURCE_TYPE, "Noise"), "set_noise", "get_noise");
}
