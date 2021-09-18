#version 330 core

layout(location = 0) in vec3 vertex_position;

uniform mat4 matrix;
uniform float intensity;
out vec3 color;

mat2 rot_2d_mat(float degs) {
	return mat2(cos(degs), -sin(degs), sin(degs), cos(degs));
}

void main() {
	float theta = intensity * 360.0f;
	mat2 frame_rot_mat = rot_2d_mat(theta);
	float src_mag = 0.5f * sin(theta);

	vec2 red_src = vec2(0.0f, src_mag) * frame_rot_mat + 0.3f;
	vec2 blue_src = vec2(-src_mag, -src_mag) * frame_rot_mat + 0.1f;
	vec2 green_src = vec2(src_mag, -0.5f) * frame_rot_mat + 0.2f;
	vec2 white_src = vec2(-0.0f, -0.75f);

	float r = 1.0f - distance(vertex_position.xy, red_src);
	r += 1.0f - distance(vertex_position.xy, white_src);
	float g = 1.0f - distance(vertex_position.xy, green_src);
	g += 1.0f - distance(vertex_position.xy, white_src);
	float b = 1.0f - distance(vertex_position.xy, blue_src);
	b += 1.0f - distance(vertex_position.xy, white_src);

	vec3 initial_color = vec3(r, g, b);
	color = initial_color;
    gl_Position = matrix * vec4(vertex_position, 1.0); 
}
