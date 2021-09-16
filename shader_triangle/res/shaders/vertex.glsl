#version 330 core

layout(location = 0) in vec3 vertex_position;

uniform mat4 matrix;
uniform vec3 cvector;
out vec3 color;

void main() {
	vec2 red_corner = vec2(0.0f, 0.5f);
	vec2 blue_corner = vec2(-0.5f, -0.5f);
	vec2 green_corner = vec2(0.5f, -0.5f);

	float r = abs(cvector.x - distance(vertex_position.xy, red_corner));
	float g = abs(cvector.y - distance(vertex_position.xy, green_corner));
	float b = abs(cvector.z - distance(vertex_position.xy, blue_corner));
	//float r = 1.0f - distance(vertex_position.xy, red_corner);
	//float g = 1.0f - distance(vertex_position.xy, green_corner);
	//float b = 1.0f - distance(vertex_position.xy, blue_corner);

	vec3 initial_color = vec3(r, g, b);
	color = initial_color;
    gl_Position = matrix * vec4(vertex_position, 1.0); 
}
