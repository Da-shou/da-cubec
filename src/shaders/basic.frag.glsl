#version 330 core
out vec4 FragColor;

uniform vec4 global_color;

void main() {
	FragColor = global_color;
}
