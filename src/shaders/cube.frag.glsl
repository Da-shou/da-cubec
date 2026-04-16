#version 330 core
out vec4 frag_color;

in vec2 texture_coordinates;
in float camera_distance;

uniform sampler2D current_texture;
uniform vec3  fog_color;
uniform float fog_near; // distance where fog starts
uniform float fog_far; // distance where fog is 100%
uniform float fog_density;

void main() {
	vec4 color = texture(current_texture, texture_coordinates);

	// Exponential squared — thickens faster, very Minecraft-like
	float fog_factor = 1.0 - exp(-pow(fog_density * camera_distance, 2.0));
	fog_factor = clamp(fog_factor, 0.0F, 1.0F);

	frag_color = mix(color, vec4(fog_color, 1.0), fog_factor);
}
