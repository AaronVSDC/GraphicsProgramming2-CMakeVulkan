#version 450

layout(push_constant) uniform PC {
    mat4 transform;
    mat4 modelMatrix;
    uint diffuseIndex;
    uint maskIndex;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec3 fragColor;
layout(location = 3) out vec2 fragUV;

void main() {
    // world-space position & normal
    fragPos   = (pc.transform * vec4(inPosition, 1.0)).xyz;
    fragNorm  = normalize((pc.modelMatrix * vec4(inNormal, 0.0)).xyz);
    fragColor = inColor;
    fragUV    = inUV;
    // clip-space
    gl_Position = pc.transform * vec4(inPosition, 1.0);
}
