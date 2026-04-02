#version 330 core
out vec4 FragColor;
in vec3 color_value;
uniform float green;

void main() {
	FragColor = vec4(color_value.x, color_value.y, green, 1.0);
}
