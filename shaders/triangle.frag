#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inLightPos;
layout (location = 4) in vec3 inFragPos;

layout (location = 0) out vec4 outFragColor;

void main() {
    vec3 norm = normalize(inNormal);
    vec3 lightDir = normalize(inLightPos - inFragPos);
    float incidence = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = incidence * vec3(0.5f, 0.3f, 0.5f);
    vec3 result = vec3(0.1f, 0.1f, 0.1f) + diffuse * inColor;
    outFragColor = vec4(result, 1.0f);
}