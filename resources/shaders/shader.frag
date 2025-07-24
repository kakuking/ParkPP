#version 450

layout(binding = 0) uniform sampler2DArrayShadow shadowMapSampler;
layout(binding = 2) uniform sampler2DArray texSampler;

layout(location = 0) in vec3 texCoord;
layout(location = 1) in vec4 shadowCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 lightPos;
layout(location = 4) in vec3 lightColor;

layout(location = 0) out vec4 outColor;

const vec2 pcf_filter_kernel[16] = vec2[](
    vec2(0.7037, 0.1633),   vec2(0.1437, 0.3935),   vec2(0.3722, 0.2278),   vec2(0.9848, 0.2637), 
    vec2(0.455, 0.2134),    vec2(0.9123, 0.5291),   vec2(0.2051, 0.0873),   vec2(0.4228, 0.4669), 
    vec2(0.4535, 0.2709),   vec2(0.9112, 0.1083),   vec2(0.4763, 0.3127),   vec2(0.005106, 0.5974), 
    vec2(0.8151, 0.8786),   vec2(0.797, 0.4556),    vec2(0.6322, 0.3194),   vec2(0.2957, 0.9349)
);

float shadow_pcf(vec3 projCoords, float z_depth, float layer) {
    float shadow = 0.0;
    float texelSize = 1.0 / textureSize(shadowMapSampler, 0).x;
    float kernel_radius = 4.0;

    float bias = 0.003;
    float biased_z = z_depth - bias;

    for (int i = 0; i < 16; ++i) {
        vec2 offset = (pcf_filter_kernel[i] * 2.0 - 1.0) * kernel_radius * texelSize;

        shadow += texture(shadowMapSampler, vec4(projCoords.xy + offset, layer, biased_z));
    }

    return shadow / 16.0;
}

void main() {
    // Base texture color
    vec4 baseColor = texture(texSampler, texCoord);

    // Convert from clip space to [0, 1]
    vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
    float z_depth = projCoords.z;
    projCoords = projCoords * 0.5 + 0.5;

    // Early discard for out-of-bounds coords
    float shadowFactor = 1.0;
    if (projCoords.x >= 0.0 && projCoords.x <= 1.0 &&
        projCoords.y >= 0.0 && projCoords.y <= 1.0 &&
        projCoords.z >= 0.0 && projCoords.z <= 1.0) {
        
        float shadowSample = shadow_pcf(projCoords, z_depth, 0.0);
        shadowFactor = mix(0.3, 1.0, shadowSample); // shadowed vs lit blend
    }

    // Lighting based on normal
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(-lightPos);

    float NdotL = max(dot(normal, -lightDir), 0.0);  // Directional light
    float diffuse = mix(0.3, 1.0, NdotL);            // Optional ambient

    vec3 temp = lightColor * baseColor.rgb * diffuse * shadowFactor;
    outColor = vec4(temp, baseColor.a);
}