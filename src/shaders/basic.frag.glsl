#version 330 core
out vec4 FragColor;
in vec2 texture_coordinates;

uniform sampler2D current_texture;

void main() {
	FragColor = texture(current_texture, texture_coordinates);
}
