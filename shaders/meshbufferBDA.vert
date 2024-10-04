#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec3 outLightPos;
layout (location = 4) out vec3 outFragPos;

struct Vertex {
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 color;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer{
    Vertex vertices[];
};

layout(push_constant) uniform constants {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 lightPos;
    VertexBuffer vertexBuffer;
} PushConstants;

void main() {
    Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

    gl_Position = PushConstants.proj * PushConstants.view * PushConstants.model * vec4(v.position, 1.0);
    outColor = v.color.xyz;
    outUV.x = v.uv_x;
    outUV.y = v.uv_y;
    outNormal = v.normal;
    outLightPos = PushConstants.lightPos;
    outFragPos = vec3(PushConstants.model * vec4(v.position, 1.0));
}