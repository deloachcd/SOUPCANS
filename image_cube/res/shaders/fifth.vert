#version 430

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;
layout(location = 2) in vec2 texture_coord;

uniform mat4 model;
uniform mat4 rotation;

out vec3 color;
out vec2 tex_coord;

void main() {
    color = vertex_color;
    tex_coord = texture_coord;
    gl_Position = model * rotation * vec4(vertex_position, 1.0); 
}
