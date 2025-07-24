    #version 450

    layout(location = 0) in vec3 inPosition;
    layout(location = 1) in float u;
    layout(location = 2) in vec3 inColor;
    layout(location = 3) in float v;
    layout(location = 4) in vec3 inNormal;
    layout(location = 5) in float inMaterialID;

    layout(location = 0) out vec3 outTexCoord;
    layout(location = 1) out vec4 outShadowCoord;
    layout(location = 2) out vec3 outFragNormal;
    layout(location = 3) out vec3 outLightPos;
    layout(location = 4) out vec3 outLightColor;

    layout(set = 0, binding = 1) readonly buffer ModelMatrices {
        mat4 model_matrices[];
    } ubo;

    layout(push_constant) uniform Constants {
        mat4 proj;
        mat4 view;
        mat4 light_pv;
        vec4 light_pos;
        vec4 light_color;
    } pc;

    void main() {
        int model_matrix_idx = int(inColor.b);
        mat4 model = ubo.model_matrices[model_matrix_idx];

        mat4 modelViewProj = pc.proj * pc.view * model;
        mat4 lightMatrix = pc.light_pv * model;

        gl_Position = modelViewProj * vec4(inPosition, 1.0);
        outShadowCoord = lightMatrix * vec4(inPosition, 1.0);

        outTexCoord = vec3(u, v, inMaterialID);  // Use extracted v here
        outFragNormal = mat3(model) * inNormal;
        outLightPos = pc.light_pos.xyz;
        outLightColor = pc.light_color.rgb;
    }