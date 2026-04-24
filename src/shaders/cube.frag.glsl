#version 330 core
out vec4 frag_color;

in vec2 texture_coordinates;
in float camera_distance;
in float frag_light;

uniform sampler2D current_texture;
uniform int max_render_distance;
uniform int render_distance;
uniform vec3 fog_color;

void main() {
	float fog_far = max_render_distance;
	float fog_near = max_render_distance * 0.75F;
	float fog_density = 2.0F;

	vec4 color = texture(current_texture, texture_coordinates);

    /* Applying light levels to each vertex */
    color.rgb *= frag_light;

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
