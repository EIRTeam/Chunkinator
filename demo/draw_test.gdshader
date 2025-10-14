shader_type canvas_item;

uniform vec2 resolution;
uniform float time;

uniform bool  show_normals;
uniform bool  cycle_methods;
uniform bool  animate_light;
uniform float slider_method;

uniform sampler2D base_map : hint_normal;
uniform sampler2D detail_map : hint_normal;

float saturate(float v)
{
    return clamp(v, 0.0, 1.0);
}

float overlay(float x, float y)
{
    if (x < 0.5)
        return 2.0*x*y;
    else
        return 1.0 - 2.0*(1.0 - x)*(1.0 - y);
}

void fragment()
{
    vec2 uv = FRAGCOORD.xy/resolution.xy;
    uv.y = 1.0 - uv.y;

    vec3 n1 = texture(base_map,   uv).xyz;
	n1.b = 0.0;
    n1 = n1*2.0 - 1.0;
	vec3 n2 = texture(detail_map, uv).xyz*2.0 - 1.0;
    vec3 n;

    float method = mod(TIME/5.0, 7.0);

    method = slider_method;

    // Linear Blending
    if (method < 1.0)
    {
        n  = normalize(n1 + n2);
    }
    // Overlay Blending
    else if (method < 2.0)
    {
        n1 = texture(base_map,   uv).xyz;
        n2 = texture(detail_map, uv).xyz;

        n.x = overlay(n1.x, n2.x);
        n.y = overlay(n1.y, n2.y);
        n.z = overlay(n1.z, n2.z);

        n = normalize(n*2.0 - 1.0);
    }
    // Partial Derivatives
    else if (method < 3.0)
    {
        n = normalize(vec3(n1.xy*n2.z + n2.xy*n1.z, n1.z*n2.z));
    }
    // Whiteout
    else if (method < 4.0)
    {
        n  = normalize(vec3(n1.xy + n2.xy, n1.z*n2.z));
    }
    // UDN
    else if (method < 5.0)
    {
        n  = normalize(vec3(n1.xy + n2.xy, n1.z));
    }
    // RNM
    else if (method < 6.0)
    {
        n1 = texture(base_map,   uv).xyz*vec3( 2,  2, 2) + vec3(-1, -1,  0);
        n2 = texture(detail_map, uv).xyz*vec3(-2, -2, 2) + vec3( 1,  1, -1);
        n  = n1*dot(n1, n2)/n1.z - n2;
    }
    // Unity
    else
    {
        mat3 nBasis = mat3(
            vec3(n1.z, n1.y, -n1.x), // +90 degree rotation around y axis
            vec3(n1.x, n1.z, -n1.y), // -90 degree rotation around x axis
            vec3(n1.x, n1.y,  n1.z));
        n = normalize(n2.x*nBasis[0] + n2.y*nBasis[1] + n2.z*nBasis[2]);
    }

    float a = animate_light ? TIME : 0.0;
    vec3 light = normalize(vec3(cos(a), sin(a), 1));

    vec3 color = saturate(dot(light, n))*vec3(1, 1, 1);

    if (show_normals)
        color = n*0.5 + 0.5;

    COLOR = vec4(color, 1);
}