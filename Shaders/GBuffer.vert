#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outColor;

layout(push_constant) uniform Push {
    mat4 transform;
    mat4 modelMatrix;
    uint materialIndex;
} push;

void main() {
    gl_Position = push.transform * vec4(inPosition, 1.0);
    mat3 normalMatrix = transpose(inverse(mat3(push.modelMatrix)));
    outNormal = normalMatrix * inNormal;
    outUV = inUV;
    outColor = inColor;
}