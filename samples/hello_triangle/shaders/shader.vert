#version 450

layout (location = 0) in vec2 inVertexPos;
layout (location = 1) in vec3 inVertexColour;

layout (location = 1) out vec4 vertexColour;

void main() {
    vertexColour = vec4(inVertexColour, 1.0f);
    gl_Position = vec4(inVertexPos, 0.5f, 1.0f);
}