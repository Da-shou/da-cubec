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

	/* Normalize distance (0.0 at fog_near, 1.0 at fog_far) */
	float dist = (camera_distance - fog_near) / (fog_far - fog_near);
	dist = clamp(dist, 0.0, 1.0);

	/* Exponential squared logic
	 * Using a density of ~3.0-4.0 ensures it hits 100% opacity at dist = 1.0 */
	float fog_factor = 1.0 - exp(-pow(dist * fog_density, 2.0));

	/* Force it to hit 1.0 at the very edge to prevent "ghosting"
	 * This tiny adjustment ensures that if dist is 1.0, fog_factor is 1.0 */
	if (dist >= 1.0) fog_factor = 1.0;

	frag_color = mix(color, vec4(fog_color, 1.0), fog_factor);
}
