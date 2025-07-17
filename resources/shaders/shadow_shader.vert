#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in float u;
layout(location = 2) in vec3 inColor;
layout(location = 3) in float v;
layout(location = 4) in vec3 normal;
layout(location = 5) in float material_id;

layout( push_constant ) uniform constants {
	mat4 MVP;
	mat4 model;
} pc;

void main() {
    gl_Position = pc.MVP * pc.model * vec4(inPosition, 1.0); 
	// gl_Position = vec4(0.5, 0.5, 0.5, 1.0);
}