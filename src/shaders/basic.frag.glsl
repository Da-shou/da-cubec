#version 330 core
out vec4 FragColor;
in vec3 color_value;

void main() {
	FragColor = vec4(color_value, 1.0);
}
