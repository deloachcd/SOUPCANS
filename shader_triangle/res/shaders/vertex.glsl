#version 330 core

// "vp" = vertex position
layout(location = 0) in vec2 triangle_vp;
layout(location = 1) in vec2 skybox_vp;
layout(location = 2) in vec2 skybox_tex_coord;

uniform mat4 matrix;
uniform float intensity;
uniform float horizontal_shift;
uniform int render_target;

out vec2 tex_coord;
out vec2 vertex_position;
out float theta;
out float hshift;

void main() {
	theta = intensity * 360.0f;
	vertex_position = skybox_vp;
	tex_coord = skybox_tex_coord;
	hshift = horizontal_shift;
	gl_Position = vec4(skybox_vp, 0.0f, 1.0f);
}
