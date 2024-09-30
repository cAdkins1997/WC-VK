#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec3 outColor;

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    vec3 color = vec3(gl_Position.r, gl_Position.g, gl_Position.b);
    outColor = color;
}