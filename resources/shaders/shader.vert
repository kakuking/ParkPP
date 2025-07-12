#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in float u;
layout(location = 2) in vec3 inColor;
layout(location = 3) in float v;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 texCoord;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout( push_constant ) uniform constants {
	mat4 model;
    vec4 idx;
} pc;

void main() {
    gl_Position = ubo.proj * ubo.view * pc.model * vec4(inPosition, 1.0);
    
    fragColor = inColor;
    texCoord = vec3(u, v, pc.idx.x);
}