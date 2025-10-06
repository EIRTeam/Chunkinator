#include "terrain_manager.h"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/image_texture_layered.hpp"
#include "godot_cpp/classes/rd_texture_format.hpp"
#include "godot_cpp/classes/rd_texture_view.hpp"
#include "godot_cpp/classes/rendering_device.hpp"
#include "godot_cpp/classes/rendering_server.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/templates/hash_set.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "terrain_generator/terrain_chunk_node.h"
#include "terrain_generator/terrain_heightmap_combine_layer.h"
#include "terrain_generator/terrain_roads.h"
#include "terrain_generator/texture_array_queue.h"
#include <limits>

void TerrainManager::_on_generation_completed() {
    Ref<TerrainFinalCombineLayer> layer = get_terrain_layer();
    HashSet<Vector2i> new_chunks;
    // Add new superchunks
    for (int i = 0; i < layer->get_chunk_count(); i++) {
        Ref<TerrainFinalCombineChunk> chunk = layer->get_chunk(i);
        const Vector2i chunk_idx = chunk->get_chunk_index();
        new_chunks.insert(chunk->get_chunk_index());
        if (!superchunks.has(chunk_idx)) {
            // Add new superchunk to tree
            TerrainSuperchunk superchunk = {
                .node = memnew(TerrainChunkNode)
            };
            add_child(superchunk.node);
            Ref<ImageTexture> height_texture = ImageTexture::create_from_image(chunk->get_height_map());
            TextureArrayQueue::TextureHandle handle = terrain_heightmap_data_array.push_texture(chunk->get_height_map());
            Ref<ImageTexture> road_sdf_texture = ImageTexture::create_from_image(chunk->get_road_sdf());
            Ref<ShaderMaterial> mat = settings->get_terrain_material();
            superchunk.node->initialize({
                .superchunk_size = chunk->get_chunk_bounds().size.x,
                .chunk_idx = chunk_idx,
                .heightmap = chunk->get_height_map(),
                .height_texture = height_texture,
                .road_sdf_texture = road_sdf_texture,
                .plane_mesh = plane_mesh,
                .material = mat,
                .heightmap_texture_handle = handle,
                .heightmap_spatial_page_texture = terrain_heightmap_spatial_page.get_texture(),
                .heightmap_texture_array = terrain_heightmap_data_array.get_texture()
            });
            
            superchunks.insert(chunk_idx, superchunk);
        }
    }
    // Delete existing ones that don't matter anymore
    HashMap<Vector2i, TerrainSuperchunk>::Iterator it = superchunks.begin();
    while (it != superchunks.end()) {
        if (!new_chunks.has(it->key)) {
            const Vector2i chunk_idx = it->key;
            ++it;
            _unload_superchunk(chunk_idx);
            continue;
        }
        ++it;
    }

    _update_spatial_page();
}

void TerrainManager::_bind_methods() {
    
}

void TerrainManager::_update_spatial_page() {
    if (superchunks.is_empty()) {
        return;
    }
    // find top left chunk

    Vector2i page_origin = Vector2i(std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max());

    for(KeyValue<Vector2i, TerrainSuperchunk> &sc : superchunks) {
        page_origin.x = MIN(sc.key.x, page_origin.x);
        page_origin.y = MIN(sc.key.y, page_origin.y);
    }

    // now... fill it up!

    terrain_heightmap_spatial_page.clear(0);

    for (KeyValue<Vector2i, TerrainSuperchunk> &sc : superchunks) {
        TextureArrayQueue::TextureHandle handle = sc.value.node->get_heightmap_texture_handle();
        terrain_heightmap_spatial_page.set_index(sc.key - page_origin, handle.texture_idx);
    }

    settings->get_terrain_material()->set_shader_parameter("spatial_map_origin", page_origin);
    terrain_heightmap_spatial_page.upload_page();
}

void TerrainManager::set_chunkinator(Ref<Chunkinator> p_chunkinator) {
    chunkinator = p_chunkinator;
    chunkinator->connect("generation_completed", callable_mp(this, &TerrainManager::_on_generation_completed));
}

void TerrainManager::_unload_superchunk(const Vector2i &p_idx) {
    auto it = superchunks.find(p_idx);
    DEV_ASSERT(it != superchunks.end());
    it->value.node->queue_free();
    terrain_heightmap_data_array.unload_handle(it->value.node->get_heightmap_texture_handle());
    superchunks.erase(p_idx);
}

void TerrainManager::set_camera_position(Vector3 p_camera_position) {
    camera_position = p_camera_position;
}

void TerrainManager::set_terrain_settings(const Ref<TerrainSettings> &p_settings) {
    settings = p_settings;
}

Ref<TerrainFinalCombineLayer> TerrainManager::get_terrain_layer() const {
    return chunkinator->get_layer("Heightmap Combine Layer");
}

Ref<TerrainRoadConnectionLayer> TerrainManager::get_road_layer() const {
    return chunkinator->get_layer("Road Connections");
}

void TerrainManager::update() {
    for (KeyValue<Vector2i, TerrainSuperchunk> &kv : superchunks) {
        kv.value.node->update(Vector2i(camera_position.x, camera_position.z));
    }   
}

void TerrainManager::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_READY: {
            terrain_heightmap_spatial_page.initialize(settings->get_geometry_chunks_spatial_map_size());
            terrain_heightmap_data_array.initialize(settings->get_geometry_chunk_heightmap_size(), settings->get_geometry_chunk_heightmap_texture_array_layer_count());
            plane_mesh = LODMesh::generate_mesh(mesh_quality);
        } break;
        case NOTIFICATION_PROCESS:  {
        } break;
    }
}

void TerrainSpatialPaging::initialize(int p_size) {
    RenderingDevice *rd = RenderingServer::get_singleton()->get_rendering_device();
    Ref<RDTextureFormat> format;
    format.instantiate();
    format->set_width(p_size);
    format->set_height(p_size);
    page_size = p_size;
    format->set_format(RenderingDevice::DATA_FORMAT_R16_SINT);
    BitField<RenderingDevice::TextureUsageBits> usage_bits = RenderingDevice::TEXTURE_USAGE_CAN_UPDATE_BIT | RenderingDevice::TEXTURE_USAGE_SAMPLING_BIT;
    format->set_usage_bits(usage_bits);

    indices.resize(sizeof(int16_t) * (p_size*p_size));
    
    int16_t *indices_write = (int16_t*)indices.ptrw();

    for (int i = 0; i < p_size*p_size; i++) {
        indices_write[i] = 0;
    }

    Ref<RDTextureView> view;
    view.instantiate();
    TypedArray<PackedByteArray> initial_data;
    initial_data.push_back(indices);
    texture_id = rd->texture_create(format, view, initial_data);
    
    DEV_ASSERT(texture_id.is_valid());

    texture.instantiate();
    texture->set_texture_rd_rid(texture_id);
    rd->set_resource_name(texture_id, "Spatial Page");
    rd->set_resource_name(RenderingServer::get_singleton()->texture_get_rd_texture(texture->get_rid()), "Spatial Page RD Texture");
}

void TerrainSpatialPaging::set_index(Vector2i p_position, int p_value) {
    ((int16_t*)indices.ptrw())[p_position.x + p_position.y * page_size] = p_value;
}

void TerrainSpatialPaging::upload_page() {
    RenderingDevice *rd = RenderingServer::get_singleton()->get_rendering_device();
    rd->texture_update(texture_id, 0, indices);
}

void TerrainSpatialPaging::clear(int p_clear_value) {
    int16_t *indices_ptr = (int16_t*)indices.ptrw();

    for (int i = 0; i < page_size*page_size; i++) {
        indices_ptr[i] = p_clear_value;
    }
}

Ref<Texture2D> TerrainSpatialPaging::get_texture() const {
    return texture;
}

TerrainSpatialPaging::~TerrainSpatialPaging() {
    RenderingDevice *rd = RenderingServer::get_singleton()->get_rendering_device();
    if (texture_id.is_valid()) {
        rd->free_rid(texture_id);
    }
}
