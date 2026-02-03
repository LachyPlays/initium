#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 rotation;
    mat4 view;
    mat4 proj;
    mat4 mvp;
} UBO;

layout (location = 0) in vec3 inVertexPos;
layout (location = 1) in vec3 inVertexNormal;
layout (location = 2) in vec2 inUV;

layout (location = 1) out vec4 fragNormal;
layout (location = 2) out vec2 fragUV;

void main() {
    fragNormal = UBO.rotation * vec4(inVertexNormal, 1.0f);
    fragUV = inUV;

    gl_Position = UBO.mvp * vec4(inVertexPos, 1.0f);
}