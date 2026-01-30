#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 mvp;
} UBO;

layout (location = 0) in vec3 inVertexPos;
layout (location = 1) in vec2 inVertexUV;

layout (location = 1) out vec2 fragUV;

void main() {
    gl_Position = UBO.mvp * vec4(inVertexPos, 1.0f);

    fragUV = inVertexUV;
}