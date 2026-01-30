#version 450

layout(location = 0) out vec4 outColor;

layout(location = 1) in vec2 fragUV;

layout (binding = 1) uniform sampler2D texSampler;

void main() {
    outColor = texture(texSampler, fragUV);
}