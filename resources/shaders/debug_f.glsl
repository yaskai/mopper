#version 330

in vec3 frag_normal;
out vec4 final_color;

void main() {
	final_color = vec4(normalize(frag_normal) * 0.5 + 0.5, 1.0);
}
