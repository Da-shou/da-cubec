#version 330 core
out vec4 FragColor;
in vec3 color_value;
in vec2 texture_coordinates;

uniform sampler2D current_texture;

void main() {
	FragColor = texture(current_texture, texture_coordinates) * vec4(color_value, 1.0);
}
