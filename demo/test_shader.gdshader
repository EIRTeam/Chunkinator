shader_type spatial;

#include "shaderlib/blend.gdshaderinc"
#include "shaderlib/noise.gdshaderinc"

uniform sampler2D albedo : source_color, filter_linear_mipmap_anisotropic;
uniform sampler2D normal : hint_normal;
uniform sampler2D roughness: hint_roughness_gray;

uniform sampler2D extra_detail_noise : repeat_enable;

global uniform sampler2DArray terrain_height_texture_array;
global uniform usampler2D terrain_spatial_page;
uniform ivec2 spatial_map_origin;
uniform vec2 superchunk_size;
uniform int lod_max_depth;

instance uniform int lod_depth;
instance uniform int side_size;
instance uniform int lod_mask;

varying vec2 corrected_uv;
varying vec2 road_sdf_uv;
varying float distance_to_road;
varying vec2 world_pos;
varying float height;
uniform int vertices_per_side = 19;
const int LOD_REDUCTION_MASK_N = 1;
const int LOD_REDUCTION_MASK_S = 2;
const int LOD_REDUCTION_MASK_E = 4;
const int LOD_REDUCTION_MASK_W = 8;

uniform int SHADOW_SAMPLE_STEPS = 32;
uniform float SHADOW_SAMPLE_DIST = 1000.0;
uniform float softness = 1.0;
uniform float shadow_pow = 1.0;
uniform float terrain_uv_scale = 0.01;
uniform float terrain_noise_scale = 0.01;
uniform float layers_count = 5.0;

vec4 get_lod_debug_color(int p_lod_depth, int p_lod_max_depth) {
	return mix(vec4(0.0, 1.0, 0.0, 1.0), vec4(1.0, 0.0, 0.0, 1.0), float(p_lod_depth) / float(lod_max_depth));
}

vec3 sample_height(vec2 p_world_pos, vec2 p_uv_min, vec2 p_uv_max, out bool r_was_valid) {
	vec2 superchunk_pos = p_world_pos / superchunk_size;
	ivec2 superchunk = ivec2(floor(superchunk_pos));
	ivec2 superchunk_in_spatial_map = superchunk - spatial_map_origin;
	uint superchunk_layer_idx = uint(texelFetch(terrain_spatial_page, superchunk_in_spatial_map, 0).r);
	vec2 new_superchunk_origin = floor(superchunk_pos) * superchunk_size;
	vec2 adjusted_uv = (p_world_pos - new_superchunk_origin) / superchunk_size;
	vec2 uv = mix(p_uv_min, p_uv_max, adjusted_uv);
	//return uv;
	r_was_valid = int(superchunk_layer_idx) != 0;
	return vec3(uv, texture(terrain_height_texture_array, vec3(uv, float(superchunk_layer_idx))).r);
	//return texture(heightmap_textures_array, vec3(uv, float(superchunk_layer_idx))).r;
}

float sample_shadows(vec2 p_world_pos, vec2 p_uv_min, vec2 p_uv_max, float p_starting_height, vec3 p_light_direction) {
	vec2 light_dir_2d = normalize(vec2(p_light_direction.x, p_light_direction.z));
	float light_slope = p_light_direction.y / length(p_light_direction.xz);
	float shadow = 0.0;

	for (int i = 0; i < SHADOW_SAMPLE_STEPS; i++) {
		float p = float(i+1) / float(SHADOW_SAMPLE_STEPS);
		p = pow(p, shadow_pow);
		vec2 new_sample_pos = (light_dir_2d*p*SHADOW_SAMPLE_DIST) + p_world_pos;
		bool was_valid = false;
		float new_height = sample_height(new_sample_pos, p_uv_min, p_uv_max, was_valid).b;
		float expected_height = (light_slope * p * SHADOW_SAMPLE_DIST) + p_starting_height;
        if (was_valid) {
            // Penumbra size grows with distance due to sun's angular size
            float effective_softness = softness + p * SHADOW_SAMPLE_DIST * tan(0.009);
            // Height difference for shadow contribution
            float height_diff = new_height - expected_height;
            // Smooth transition using smoothstep
            float shadow_contrib = smoothstep(-effective_softness, effective_softness, height_diff);
            // Accumulate shadow, attenuating with distance for realism
            shadow = max(shadow, shadow_contrib * (1.0 - p * 0.05)); // Slight distance falloff
        } else {
            break; // Stop if the sample is invalid
        }

		if (!was_valid) {
			break;
		}
	}

	return shadow;
}

void vertex() {
	bool is_skirt_vertex = VERTEX_ID >= (vertices_per_side*vertices_per_side);

	//COLOR.x = float(texture(heightmap_spatial_map, UV).r);
	//COLOR.g = float(texture(heightmap_textures_array, vec3(0.0, 0.0, 0.0)).r);

	if (!is_skirt_vertex) {
		if (bool(lod_mask & LOD_REDUCTION_MASK_N)) {
			if (VERTEX_ID < vertices_per_side && VERTEX_ID >= 0)
			{
				int vertex_x = VERTEX_ID % vertices_per_side;
				int vertex_y = VERTEX_ID / vertices_per_side;
				if (vertex_y == 0 && vertex_x % 2 != 0)
				{
					float vert_coord_curr_x = VERTEX.x;
					float vert_coord_prev_x = float(vertex_x-1) / float(vertices_per_side-1);
					VERTEX.x = vert_coord_prev_x;
				}
			}
		}
		if (bool(lod_mask & LOD_REDUCTION_MASK_S)) {
			int vertex_x = VERTEX_ID % vertices_per_side;
			int vertex_y = VERTEX_ID / vertices_per_side;
			if (vertex_y == vertices_per_side-1 && vertex_x % 2 != 0)
			{
				float vert_coord_curr_x = VERTEX.x;
				float vert_coord_prev_x = float(vertex_x+1) / float(vertices_per_side-1);
				VERTEX.x = vert_coord_prev_x;
			}
		}
		if (bool(lod_mask & LOD_REDUCTION_MASK_E)) {
			int vertex_x = VERTEX_ID % vertices_per_side;
			if (vertex_x == vertices_per_side-1)
			{
				int vertex_y = VERTEX_ID / vertices_per_side;
				if (vertex_y % 2 != 0)
				{
					float vert_coord_next_y = float(vertex_y+1) / float(vertices_per_side-1);
					VERTEX.z = vert_coord_next_y;
				}
			}
		}
		if (bool(lod_mask & LOD_REDUCTION_MASK_W)) {
			int vertex_x = VERTEX_ID % vertices_per_side;
			if (vertex_x == 0)
			{
				int vertex_y = VERTEX_ID / vertices_per_side;
				if (vertex_y % 2 != 0)
				{
					float vert_coord_prev_y = float(vertex_y-1) / float(vertices_per_side-1);
					VERTEX.z = vert_coord_prev_y;
				}
			}
		}
	}

	vec2 tex_size = 1.0 / vec2(textureSize(terrain_height_texture_array, 0).xy);
	vec2 uv_min = tex_size * 0.5;
	vec2 uv_max = vec2(1.0, 1.0) - tex_size * 0.5;

	VERTEX.xz *= float(side_size);
	world_pos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xz;
	//vec2 adjusted_uv = (world_pos - superchunk_origin) / superchunk_size;
	//corrected_uv = mix(uv_min, uv_max, adjusted_uv);
	bool was_valid = false;
	//COLOR.r = height;
	//VERTEX.y = height;
	//VERTEX.y = height;
	height = sample_height(world_pos, uv_min, uv_max, was_valid).b;
	VERTEX.y = height;

	if (is_skirt_vertex) {
		VERTEX.y -= 100.0;
	}
	//NORMAL = normals_and_height.rgb;
	/*
	            const Vector2 point_to_sample = Vector2(sample_x, rect.position.y + rect.size.y * (y / (float)(heightmap_size-1)));
            double height = layer->sample_height(point_to_sample);

            /*const double SAMPLE_NUDGE = 50.0;

            const double hL = layer->sample_height(point_to_sample - Vector2(SAMPLE_NUDGE, 0.0));
            const double hR = layer->sample_height(point_to_sample + Vector2(SAMPLE_NUDGE, 0.0));
            const double hD = layer->sample_height(point_to_sample - Vector2(0.0, SAMPLE_NUDGE));
            const double hU = layer->sample_height(point_to_sample + Vector2(0.0, SAMPLE_NUDGE));

            // deduce terrain normal
            Vector3 normal;
            normal.x = hL - hR;
            normal.y = SAMPLE_NUDGE*2.0;
            normal.z = hD - hU;
            normal.normalize();*/

	const float SAMPLE_NUDGE = 50.0;
	vec2 sample_nudge_vec = vec2(SAMPLE_NUDGE, 0.0);

float hL = sample_height(world_pos - sample_nudge_vec.xy, uv_min, uv_max, was_valid).b;
	if (!was_valid) {
		NORMAL.x = 1.0;
	} else {
    	float hR = sample_height(world_pos + sample_nudge_vec.xy, uv_min, uv_max, was_valid).b;
		if (!was_valid) {
			NORMAL.x = 1.0;
		} else {
    		float hD = sample_height(world_pos - sample_nudge_vec.yx, uv_min, uv_max, was_valid).b;
			if (!was_valid) {
				NORMAL.x = 1.0;
			} else {
    			float hU = sample_height(world_pos + sample_nudge_vec.yx, uv_min, uv_max, was_valid).b;
				if (!was_valid) {
					NORMAL.x = 1.0;
				} else {
					NORMAL = vec3(0.0);
					NORMAL.x = hL - hR;
					NORMAL.y = SAMPLE_NUDGE * 2.0;
					NORMAL.z = hD - hU;

				    NORMAL = normalize(NORMAL); // Base normal
				}
			}

		}
	}

    TANGENT = normalize(vec3(1.0, 0.0, 0.0) - dot(vec3(1.0, 0.0, 0.0), NORMAL) * NORMAL); // Orthogonalize tangent
    BINORMAL = normalize(cross(NORMAL, TANGENT)); // Compute bitangent
	COLOR.rgb = NORMAL;

	/*vec2 road_sdf_tex_size = 1.0 / vec2(textureSize(road_sdf, 0));
	vec2 road_sdf_uv_min = road_sdf_tex_size * 0.5;
	vec2 road_sdf_uv_max = vec2(1.0, 1.0) - road_sdf_tex_size * 0.5;
	vec2 road_sdf_corrected_uv = mix(road_sdf_uv_min, road_sdf_uv_max, adjusted_uv);
	road_sdf_uv = road_sdf_corrected_uv;*/
	//distance_to_road = texture(road_sdf, road_sdf_uv).r;
	//COLOR = get_lod_debug_color(lod_depth, lod_max_depth);
}

void fragment() {
	/*vec2 tex_size = 1.0 / vec2(textureSize(heightmap, 0));
	vec2 uv_min = tex_size * 0.5;
	vec2 uv_max = vec2(1.0, 1.0) - tex_size * 0.5;

	vec2 terrain_texture_uv = world_pos * 0.01;
	//extra_detail_noise_normal.xy = extra_detail_noise_normal.xy * 2.0 - 1.0;
	//extra_detail_noise_normal.z = sqrt(max(0.0, 1.0 - dot(extra_detail_noise_normal.xy, extra_detail_noise_normal.xy))); //always ignore Z, as it can be RG packed, Z may be pos/neg, etc.

	vec3 terrain_normal = texture(heightmap, corrected_uv).rgb;*/


	//NORMAL = (VIEW_MATRIX * vec4(terrain_normal, 0.0)).rgb;
	//float dist = texture(road_sdf, road_sdf_uv).r;
	ALBEDO = vec3(0.5);

	//ALBEDO = texture(albedo, terrain_texture_uv).rgb;// * COLOR.rgb;
	//vec3 texture_normal = texture(normal, terrain_texture_uv).rgb;
	//ALBEDO = blend_normals_rnm(texture_normal, extra_detail_noise_normal * 2.0 - 1.0);
	//NORMAL_MAP = texture_normal;
	//NORMAL_MAP = blend_normals_rnm(extra_detail_noise_normal, texture(normal, terrain_texture_uv).rgb);
	//ROUGHNESS = texture(roughness, terrain_texture_uv).r;

	float sampler_noise = texture(extra_detail_noise, world_pos * terrain_noise_scale).r;

    InterpNodes2 interpNodes = GetNoiseInterpNodes(sampler_noise * layers_count);
    float moment2 = 0.0;
	ALBEDO = vec3(0.0);
	NORMAL_MAP = vec3(0.0);
	ROUGHNESS = 0.0;
    for(int i = 0; i < 2; i++)
    {
        float weight = interpNodes.weights[i];
        moment2 += weight * weight;
		vec2 sample_uv = GetTextureSampleUV(world_pos * terrain_uv_scale, 1.0, interpNodes.seeds[i]);
        ALBEDO += texture(albedo, sample_uv).rgb * weight;
		NORMAL_MAP += texture(normal, sample_uv).rgb * weight;
		ROUGHNESS += texture(roughness, sample_uv).r * weight;
    }

	/*if (dist < 150.0) {
		ALBEDO.r = 0.0;
	}*/
	//if (height < 0.5) {
		//ALBEDO.rgb = vec3(1.0, 0.0, 0.0);
	//} else {
		//ALBEDO.rgb = vec3(height);
	//ALBEDO.rg = UV;
	//}

	//ALBEDO = COLOR.rgb;
}
void light() {
	float att = ATTENUATION;
	if (LIGHT_IS_DIRECTIONAL) {
		vec2 tex_size = 1.0 / vec2(textureSize(terrain_height_texture_array, 0).xy);
		vec2 uv_min = tex_size * 0.5;
		vec2 uv_max = vec2(1.0, 1.0) - tex_size * 0.5;
		vec3 light_vector = (INV_VIEW_MATRIX * vec4(LIGHT, 0.0)).xyz;
		bool valid = false;
		float new_height = sample_height(world_pos, uv_min, uv_max, valid).b;
		float shadow = sample_shadows(world_pos, uv_min, uv_max, new_height, light_vector);
		att = min(1.0 - shadow, att);
	}

    DIFFUSE_LIGHT += clamp(dot(NORMAL, LIGHT), 0.0, 1.0) * att * LIGHT_COLOR / PI;

}


//void light() {
//	// Called for every pixel for every light affecting the material.
//	// Uncomment to replace the default light processing function with this one.
//}
