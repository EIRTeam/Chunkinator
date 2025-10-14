#ifndef SHADERLIB_NOISE_H
#define SHADERLIB_NOISE_H

struct InterpNodes2
{
    vec2 seeds;
    vec2 weights;
};
InterpNodes2 GetNoiseInterpNodes(float smoothNoise)
{
    vec2 globalPhases = vec2(smoothNoise * 0.5) + vec2(0.5, 0.0);
    vec2 phases = fract(globalPhases);
    vec2 seeds = floor(globalPhases) * 2.0 + vec2(0.0, 1.0);
    vec2 weights = min(phases, vec2(1.0f) - phases) * 2.0;
    return InterpNodes2(seeds, weights);
}


vec3 hash33( vec3 p )
{
	p = vec3( dot(p,vec3(127.1,311.7, 74.7)),
			  dot(p,vec3(269.5,183.3,246.1)),
			  dot(p,vec3(113.5,271.9,124.6)));

	return fract(sin(p)*43758.5453123);
}

vec2 GetTextureSampleUV(vec2 p_pos, float p_freq, float p_seed)
{
    vec3 hash = hash33(vec3(p_seed, 0.0, 0.0));
    float ang = hash.x * 2.0 * PI;
    mat2 rotation = mat2(vec2(cos(ang), sin(ang)), vec2(-sin(ang), cos(ang)));
    
    vec2 uv = rotation * p_pos * p_freq + hash.yz;
    return uv;
}

#endif