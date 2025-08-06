
//Geometry.vert
#version 450

layout(push_constant) uniform PC {
    mat4 transform;
    mat4 modelMatrix;
    uint albedoIndex;
    uint normalIndex;
    uint metalRoughIndex;
    uint occlusionIndex;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBiTangent;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec3 fragColor;
layout(location = 3) out vec2 fragUV;
layout(location = 4) out vec3 fragTangent;
layout(location = 5) out vec3 fragBiTangent;

void main() {

    // clip-space
    gl_Position = pc.transform * vec4(inPosition, 1.0);

    // world-space position & normal
    fragPos   = (pc.modelMatrix * vec4(inPosition, 1.0)).xyz;
    mat3 normalMatrix = transpose(inverse(mat3(pc.modelMatrix)));
    fragNorm  = normalize((normalMatrix * inNormal));
    fragTangent   = normalize((pc.modelMatrix * vec4(inTangent,   0.0)).xyz);
    fragBiTangent = normalize((pc.modelMatrix * vec4(inBiTangent, 0.0)).xyz);
    fragColor = inColor;
    fragUV    = inUV;

}
