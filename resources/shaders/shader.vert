#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in float u;
layout(location = 2) in vec3 inColor;
layout(location = 3) in float v;
layout(location = 4) in vec3 normal;
layout(location = 5) in float material_id;

layout(location = 0) out vec3 texCoord;
layout(location = 1) out vec4 shadowCoord; // Pass light-space position to fragment
layout(location = 2) out vec3 fragNormal;

layout(binding = 1) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform constants {
    mat4 lightMVP;
    mat4 model;
} pc;

void main() {
    mat4 modelViewProj = ubo.proj * ubo.view * pc.model;
    mat4 lightMatrix = pc.lightMVP * pc.model;

    gl_Position = modelViewProj * vec4(inPosition, 1.0);
    shadowCoord = lightMatrix * vec4(inPosition, 1.0); // <- for shadow map lookup
    texCoord = vec3(u, v, material_id);

    fragNormal = mat3(pc.model) * normal;
}