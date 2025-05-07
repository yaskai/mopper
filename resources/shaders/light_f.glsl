#version 330

#define MAX_LIGHTS 12 

// Input from vertex shader
in vec2 frag_texcoord;	// Texture coordinates
in vec3 frag_worldpos;
in vec3 frag_normal;

// Uniforms (set from C code)
uniform sampler2D texture0;		// Texture to use
uniform vec4 col_diffuse;		// Base color(white by default)
uniform vec3 light_pos;			// Light position(set in game code)
uniform float light_range;		// How far can light travel

uniform int light_enabled[MAX_LIGHTS];
uniform vec3 light_positions[MAX_LIGHTS];
uniform vec3 light_colors[MAX_LIGHTS];
uniform float light_ranges[MAX_LIGHTS];
uniform int light_count;

uniform float time;

// Output color
out vec4 final_color;

float noise(vec2 uv, float t) {
    return fract(sin(dot(uv, vec2(12.9898, 78.233)) + (t * 0.0005)) * 43758.5453);
}

void main() {
	vec3 normal = normalize(frag_normal);

	// Sample the texture at current UV coordinates
	vec4 tex_color = texture(texture0, frag_texcoord);
	vec4 tint = tex_color;

	vec3 total_light = vec3(0);
	for(int i = 0; i < light_count; i++) {
		if(light_enabled[i] == 1) {
			vec3 light_dir = normalize(light_positions[i] - frag_worldpos);
			float dist = distance(light_positions[i], frag_worldpos);

			float breathe = sin(time * 2.0 + float(i) * 3.14) * 0.05 + 1.0;
			
			float dyn_range = light_ranges[i] * breathe;

			float attenuation = 1.0 - smoothstep(0.0, dyn_range, dist);
			float diffuse = max(dot(normal, light_dir), 0.0);

			total_light += light_colors[i] * diffuse * attenuation;
		}
	}

	vec3 ambient_light = vec3(0.01, 0.01, 0.01);  // Small ambient light
	total_light += ambient_light;

	vec3 lit = tint.rgb * total_light;

	float dither = noise(frag_worldpos.xz, time) * 0.025;
	vec3 quantized = ((lit + dither) * 255.0) / 255.0;

	final_color = vec4(quantized, 1.0);
}

