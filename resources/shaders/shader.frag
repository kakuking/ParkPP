#version 450

layout(binding = 0) uniform sampler2DArrayShadow shadowMapSampler;
layout(binding = 2) uniform sampler2DArray texSampler;

layout(location = 0) in vec3 texCoord;
layout(location = 1) in vec4 shadowCoord;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main() {
    // Base texture color
    vec4 baseColor = texture(texSampler, texCoord);

    // Convert from clip space to [0, 1]
    vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
    float z_depth = projCoords.z;
    projCoords = projCoords * 0.5 + 0.5;
    // vec2 projCoordsUV = projCoords.xy * 0.5 + 0.5;

    // Early discard for out-of-bounds coords
    float shadowFactor = 1.0;
    if (projCoords.x >= 0.0 && projCoords.x <= 1.0 &&
        projCoords.y >= 0.0 && projCoords.y <= 1.0 &&
        projCoords.z >= 0.0 && projCoords.z <= 1.0) {
        
        float shadowSample = texture(shadowMapSampler, vec4(projCoords.xy, 0.0, z_depth));
        shadowFactor = shadowSample > 0.0 ? 1.0 : 0.3;
    }

    // Lighting based on normal
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(vec3(0.0, -5.0, -5.0));  // or hardcode like vec3(0.5, -1.0, 0.3)

    float NdotL = max(dot(normal, -lightDir), 0.0);  // Directional light
    float diffuse = mix(0.3, 1.0, NdotL);            // Optional ambient

    outColor = vec4(baseColor.rgb * diffuse * shadowFactor, 1.0);
}