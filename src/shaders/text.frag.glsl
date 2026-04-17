#version 330 core
in vec2 tex_coords;
out vec4 frag_color;

uniform sampler2D text_texture;
uniform vec3 text_color;
uniform float force_alpha; // -1.0 = sample texture, otherwise use as flat alpha

void main() {
	if (force_alpha >= 0.0) {
		frag_color = vec4(text_color, force_alpha); // flat color for background
	} else {
		float alpha = texture(text_texture, tex_coords).r;
		frag_color = vec4(text_color, alpha);        // normal text rendering
	}
}
