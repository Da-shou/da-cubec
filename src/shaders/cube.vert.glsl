#version 330 core

layout (location = 0) in uint packed_data;

out vec2 texture_coordinates;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	/* Unpacking the vertex data packed into a 32 bit unsigned int. */
	float x = float(packed_data & 0x01Fu);
	float z = float((packed_data >> 5u) & 0x01Fu);
	float y = float((packed_data >> 10u) & 0x3FFu);
	float u = float((packed_data >> 20u) & 0x007u) * 0.25;
	float v = float((packed_data >> 23u) & 0x007u) * 0.25;

	gl_Position = projection * view * model * vec4(x, y, z, 1.0);
	texture_coordinates = vec2(u,v);
}
