#version 330 core

in float theta;
in vec2 vertex_position;
in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D sky_texture;

#define TRIANGLE_MAGNITUDE 0.5f

mat2 rot_2d_mat(float degs) {
	return mat2(cos(degs), -sin(degs), sin(degs), cos(degs));
}

vec3 compute_position_rgb(vec2 vposition) {
	mat2 frame_rot_mat = rot_2d_mat(theta);
	float src_mag = 0.5f * sin(theta);
	vec2 red_src = vec2(0.0f, src_mag) * frame_rot_mat + 0.3f;
	vec2 blue_src = vec2(-src_mag, -src_mag) * frame_rot_mat + 0.1f;
	vec2 green_src = vec2(src_mag, -0.5f) * frame_rot_mat + 0.2f;
	vec2 white_src = vec2(-0.0f, -0.75f);

	float r = 1.0f - distance(vposition, red_src);
	r += 1.0f - distance(vposition, white_src);

	float g = 1.0f - distance(vposition, green_src);
	g += 1.0f - distance(vposition, white_src);

	float b = 1.0f - distance(vposition, blue_src);
	b += 1.0f - distance(vposition, white_src);

	return vec3(r, g, b);
}

bool position_in_triangle(vec2 vposition) {

	if (vposition.y < -TRIANGLE_MAGNITUDE) {
		return false;
	} else if (vposition.x > 0) {
		return vposition.y <= TRIANGLE_MAGNITUDE - (2 * vposition.x);
	} else {
		return vposition.y <= TRIANGLE_MAGNITUDE + (2 * vposition.x);
	}
}


void main() {
    //FragColor = color;
	if (position_in_triangle(vertex_position)) {
		frag_color = vec4(compute_position_rgb(vertex_position), 1.0f); 
	} else {
		frag_color = texture(sky_texture, tex_coord);
	}
}
