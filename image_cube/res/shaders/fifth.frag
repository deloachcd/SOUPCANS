#version 430

in vec3 color;
in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D cubeTexture; 

void main() {
    // frag_color = vec4(color, 1.0); 
    frag_color = texture(cubeTexture, tex_coord); 
    // frag_color = texture(inaTexture, tex_coord) * vec4(color, 1.0); 
}
