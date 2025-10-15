shader_type spatial;

varying vec3 world_pos;

uniform sampler2D test_text: repeat_enable, source_color, filter_linear_mipmap_anisotropic;
uniform sampler2D test_text_displacement: repeat_enable, filter_linear_mipmap_anisotropic;
uniform sampler2D test_text_normal: hint_normal, filter_linear_mipmap_anisotropic;
uniform sampler2D test_text_roughness: hint_roughness_r;
uniform float blend_range = 0.1; // Controls transition smoothness (0.0 = sharp, higher = smoother)
//
// Description : Array and textureless GLSL 2D simplex noise function.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
//

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
  return mod289(((x*34.0)+10.0)*x);
}

float snoise(vec2 v)
  {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

void vertex() {
	world_pos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
	// Called for every vertex the material is visible on.
}

float saturate(float p_val) {
	return clamp(p_val, 0.0, 1.0);
}

// Compute local triangle barycentric coordinates and vertex IDs
void triangle_grid ( vec2 uv, out float w1 , out float w2 , out float w3, out ivec2 vertex1 , out ivec2 vertex2 , out ivec2 vertex3 ) {
	// Scaling of the input
	uv *= 3.464; // 2 * sqrt (3)
	// Skew input space into simplex triangle grid
	const mat2 gridToSkewedGrid = mat2 (vec2(1.0 , 0.0), vec2(-0.57735027 , 1.15470054)) ;
	vec2 skewedCoord = gridToSkewedGrid * uv ;
	// Compute local triangle vertex IDs and local barycentric cordinates
	ivec2 baseId = ivec2 ( floor ( skewedCoord ));
	vec3 temp = vec3 ( fract ( skewedCoord ) , 0) ;
	temp .z = 1.0 - temp . x - temp .y;
	if ( temp . z > 0.0) {
		w1 = temp .z;
		w2 = temp .y;
		w3 = temp .x;
		vertex1 = baseId ;
		vertex2 = baseId + ivec2 (0 , 1) ;
		vertex3 = baseId + ivec2 (1 , 0) ;
	} else {
		w1 = - temp .z ;
		w2 = 1.0 - temp .y;
		w3 = 1.0 - temp .x;
		vertex1 = baseId + ivec2 (1 , 1) ;
		vertex2 = baseId + ivec2 (1 , 0) ;
		vertex3 = baseId + ivec2 (0 , 1) ;
	}
}

vec2 hash ( vec2 p)
{
return fract ( sin (( p ) * mat2 (vec2(127.1 , 311.7) , vec2(269.5 , 183.3)) )*43758.5453) ;
}

struct StochasticTilingData {
	vec2 uvs[3];
	vec2 derivatives[2];
	float weights[3];
};

StochasticTilingData stochastic_tiling_and_blending ( vec2 uv )
{
	// Get triangle info
	float w1 , w2 , w3 ;
	ivec2 vertex1 , vertex2 , vertex3 ;
	triangle_grid (uv , w1 , w2 , w3 , vertex1 , vertex2 , vertex3 );
	// Assign random offset to each triangle vertex
	vec2 uv1 = uv + hash ( vec2(vertex1) );
	vec2 uv2 = uv + hash ( vec2(vertex2) );
	vec2 uv3 = uv + hash ( vec2(vertex3) );
	// Precompute UV derivatives
	vec2 duvdx = dFdx ( uv ) ;
	vec2 duvdy = dFdy ( uv ) ;
	// Fetch input
	vec3 I1 = textureGrad ( test_text , uv1 , duvdx , duvdy ). rgb ;
	vec3 I2 = textureGrad ( test_text , uv2 , duvdx , duvdy ). rgb ;
	vec3 I3 = textureGrad ( test_text , uv3 , duvdx , duvdy ). rgb ;
	// Linear blending
	vec3 color = w1 * I1 + w2 * I2 + w3 * I3 ;
	StochasticTilingData data;
	data.uvs[0] = uv1;
	data.uvs[1] = uv2;
	data.uvs[2] = uv3;
	data.derivatives[0] = duvdx;
	data.derivatives[1] = duvdy;
	data.weights[0] = w1;
	data.weights[1] = w2;
	data.weights[2] = w3;
	return data;
}

float[3] height_blend(float p_height1, float p_height2, float p_height3) {
    float[3] weights;
	weights[0] = smoothstep(0.0, blend_range, p_height1 - max(p_height2, p_height3));
    float totalWeight = weights[0];

    // Compute weight for texture 2
    weights[1] = smoothstep(0.0, blend_range, p_height2 - max(p_height1, p_height3));
    totalWeight += weights[1];

    // Compute weight for texture 3
    weights[2] = smoothstep(0.0, blend_range, p_height3 - max(p_height1, p_height2));
    totalWeight += weights[2];

    if (totalWeight > 0.0) {
        weights[0] /= totalWeight;
        weights[1] /= totalWeight;
        weights[2] /= totalWeight;
    } else {
        // If all weights are 0, assign equal weights or handle edge case
        weights[0] = weights[1] = weights[2] = 1.0 / 3.0;
    }

	return weights;
}

void fragment() {
	// Called for every pixel the material is visible on.
	StochasticTilingData data = stochastic_tiling_and_blending(world_pos.xz);

	ALBEDO = vec3(0.0);
	vec3 colors[3];
	float height[3];
	vec3 normals[3];
	float roughness[3];
	for (int i = 0; i < 3; i++) {
		vec2 uv = data.uvs[i];

		vec3 col = textureGrad(test_text, data.uvs[i], data.derivatives[0], data.derivatives[1]).rgb;

		colors[i] = col;
		height[i] = textureGrad(test_text_displacement, data.uvs[i], data.derivatives[0], data.derivatives[1]).r * data.weights[i];
		normals[i] = textureGrad(test_text_normal, data.uvs[i], data.derivatives[0], data.derivatives[1]).rgb;
		roughness[i] = textureGrad(test_text_roughness, data.uvs[i], data.derivatives[0], data.derivatives[1]).r;

	}
	float weights[3] = height_blend(height[0], height[1], height[2]);

	ROUGHNESS = 0.0;
	NORMAL_MAP = vec3(0.0);
	for (int i = 0; i < 3; i++) {
		ALBEDO += colors[i] * data.weights[i];
		NORMAL_MAP += normals[i] * data.weights[i];
		ROUGHNESS += roughness[i] * data.weights[i];
	}
	//ALBEDO = vec3(height[0] * height[1] * height[2]);
}

//void light() {
//	// Called for every pixel for every light affecting the material.
//	// Uncomment to replace the default light processing function with this one.
//}
