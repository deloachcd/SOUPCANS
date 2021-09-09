#version 430

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;

uniform mat4 model;
uniform mat4 squish;
uniform mat4 rotation;

out vec3 color;

void main() {
    color = vertex_color;
    //color = vec3(1.0, 0.0, 0.0);
    gl_Position = model * squish * rotation * vec4(vertex_position, 1.0); 
}
