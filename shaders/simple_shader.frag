#version 450

layout (location = 0) in vec3 i_color;

layout (location = 0) out vec4 out_color;
void main() {
    out_color = vec4(i_color,0.0);
}