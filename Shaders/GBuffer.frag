#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform sampler2D u_Textures[];

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inColor;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;

layout(push_constant) uniform Push {
    mat4 transform;
    mat4 modelMatrix;
    uint materialIndex;
} push;

void main() {
    vec4 albedo = texture(u_Textures[push.materialIndex], inUV) * vec4(inColor, 1.0);
    outAlbedo = albedo;
    outNormal = vec4(normalize(inNormal) * 0.5 + 0.5, 1.0);
}