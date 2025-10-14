#ifndef SHADERLIB_BLEND_H
#define SHADERLIB_BLEND_H
vec3 blend_normals_rnm(vec3 n1, vec3 n2) {
	vec3 t = n1.xyz*vec3( 2,  2, 2) + vec3(-1, -1,  0);
	vec3 u = n2.xyz*vec3(-2, -2, 2) + vec3( 1,  1, -1);
	vec3 r = t*dot(t, u) / t.z - u;
	return normalize(r);
}

// stochastic texture sampling



#endif