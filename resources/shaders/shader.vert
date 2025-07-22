    #version 450

    layout(location = 0) in vec3 inPosition;
    layout(location = 1) in float u;
    layout(location = 2) in vec3 inColor;
    layout(location = 3) in float v;
    layout(location = 4) in vec3 normal;
    layout(location = 5) in float material_id;

    layout(location = 0) out vec3 texCoord;
    layout(location = 1) out vec4 shadowCoord;
    layout(location = 2) out vec3 fragNormal;
    layout(location = 3) out vec3 outLightPos;

    layout(set = 0, binding = 1) readonly buffer ModelMatrices {
        mat4 model_matrices[];
    } ubo;

    layout(push_constant) uniform Constants {
        mat4 proj;
        mat4 view;
        mat4 lightMVP;
        vec4 light_pos;
    } pc;

    void main() {
        int model_matrix_idx = int(inColor.b);
        mat4 model = ubo.model_matrices[model_matrix_idx];

        mat4 modelViewProj = pc.proj * pc.view * model;
        mat4 lightMatrix = pc.lightMVP * model;

        gl_Position = modelViewProj * vec4(inPosition, 1.0);
        shadowCoord = lightMatrix * vec4(inPosition, 1.0);

        texCoord = vec3(u, v, material_id);  // Use extracted v here
        fragNormal = mat3(model) * normal;
        outLightPos = pc.light_pos.xyz;
    }