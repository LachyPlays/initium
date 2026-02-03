#version 450

layout(binding = 1) uniform sampler2D albedoSampler;

layout(location = 0) out vec4 outColor;

layout(location = 1) in vec4 fragNormal;
layout(location = 2) in vec2 fragUV;

const vec3 ambientTerm = vec3(0.1f, 0.1f, 0.1f);
const vec3 sunDir = normalize(vec3(-1.0f, 0.0f, 0.0f));
const vec3 sunColour = vec3(1.0f, 1.0f, 1.0f);
const float sunIntensity = 1.0f;

void main() {
    vec4 albedo = texture(albedoSampler, fragUV);

    vec3 irradiance = ambientTerm;
    irradiance += sunColour * max(dot(-sunDir, vec3(fragNormal)), 0.0f);

    outColor = albedo * vec4(irradiance, 1.0f);
}