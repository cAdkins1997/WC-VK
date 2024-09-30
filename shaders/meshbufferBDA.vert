#version 450
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout (location = 0) out vec4 fragColor;

struct Vertex {
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 color;
};

layout(std430, buffer_reference, buffer_reference_align = 4) readonly buffer Position {
    Vertex vertices[];
};

layout(std430, push_constant) uniform Constants {
    mat4 worldMatrix;
    uint64_t deviceAddress;
} constants;

void main() {
    Position pos = Position(constants.deviceAddress);
    fragColor = pos.vertices[gl_VertexIndex].color;
}