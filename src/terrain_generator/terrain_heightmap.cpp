#include "terrain_heightmap.h"
#include "godot_cpp/classes/image_texture.hpp"

void TerrainHeightmapChunk::generate() {
    Ref<Image> img = Image::create_empty(DATA_SIZE, DATA_SIZE, false, Image::Format::FORMAT_RGBAF);

    print_line("Building heightmap for", get_chunk_index());

    for (int x = 0; x < DATA_SIZE; x++) {
        const double x_v = (x / (double)(DATA_SIZE-1));
        for  (int y = 0; y < DATA_SIZE; y++) {
            const double y_v = (y / (double)(DATA_SIZE-1));

            const double SAMPLE_NUDGE = 50.0;

            Vector2 sample_center = get_chunk_bounds().position + (Vector2(x_v, y_v) * get_chunk_bounds().size);
            const double hL = layer->sample_noise(sample_center - Vector2(SAMPLE_NUDGE, 0.0));
            const double hR = layer->sample_noise(sample_center + Vector2(SAMPLE_NUDGE, 0.0));
            const double hD = layer->sample_noise(sample_center - Vector2(0.0, SAMPLE_NUDGE));
            const double hU = layer->sample_noise(sample_center + Vector2(0.0, SAMPLE_NUDGE));

            // deduce terrain normal
            Vector3 normal;
            normal.x = hL - hR;
            normal.y = SAMPLE_NUDGE*2.0;
            normal.z = hD - hU;
            normal.normalize();

            img->set_pixel(x, y, Color(normal.x, normal.y, normal.z, layer->sample_noise(sample_center)));
        }
    }

    heightmap = img;
}

void TerrainHeightmapChunk::debug_draw(ChunkinatorDebugDrawer *p_debug_drawer) const {
    if (!debug_texture.is_valid()) {
        Ref<Image> debug_image = Image::create_empty(heightmap->get_width(), heightmap->get_height(), false, Image::FORMAT_RGBA8);

        for (int x = 0; x < debug_image->get_width(); x++) {
            for (int y = 0; y < debug_image->get_height(); y++) {
                float height = heightmap->get_pixel(x, y).a;
                debug_image->set_pixel(x, y, Color(height, height, height, 1.0f));
            }
        }
        const_cast<TerrainHeightmapChunk*>(this)->debug_texture = ImageTexture::create_from_image(debug_image);
    }
    p_debug_drawer->draw_texture(debug_texture, get_chunk_bounds());
}

int TerrainHeightmapLayer::get_chunk_size() const {
    return 8192;
}

Ref<ChunkinatorChunk> TerrainHeightmapLayer::instantiate_chunk() {
    Ref<TerrainHeightmapChunk> chunk;
    chunk.instantiate();
    chunk->layer = this;
    return chunk;
}

double TerrainHeightmapLayer::sample_noise(const Vector2 &p_world_position) const {
    return noise->get_noise_2dv(p_world_position * 0.01);// * 500.0;
}

TerrainHeightmapLayer::TerrainHeightmapLayer() {
    noise.instantiate();
    noise->set_frequency(0.01);
}
