#version 450

// layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 1) uniform sampler2DArray texSampler;
// layout(binding = 1) uniform sampler2D texSampler2;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    // outColor = vec4(fragColor, 1.0);
    // outColor = vec4(fragTexCoord, 0.0, 1.0);
    // outColor = texture(texSampler, fragTexCoord);
    outColor = texture(texSampler, fragTexCoord);
}