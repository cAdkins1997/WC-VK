#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec3 outFragPos;
layout (location = 4) out vec3 outLightPos;

struct Vertex {
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 color;
};

struct Frustum {
    vec4 planes[6];
};

layout(buffer_reference, std430) readonly buffer VertexBuffer {
    Vertex vertices[];
};

layout(push_constant) uniform constants {
    VertexBuffer vertexBuffer;
} PushConstants;

layout(set = 0, binding = 1) uniform SceneData {
    mat4 model;
    mat4 view;
    mat4 perspective;
    Frustum frustum;
    vec3 lightPos;
} sceneData;

void main() {
    Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

    gl_Position = sceneData.perspective * sceneData.view * sceneData.model * vec4(v.position, 1.0);
    outColor = v.color.xyz;
    outUV.x = v.uv_x;
    outUV.y = v.uv_y;
    outNormal = v.normal;
    outLightPos = sceneData.lightPos;
    outFragPos = vec3(sceneData.model * vec4(v.position, 1.0));
}