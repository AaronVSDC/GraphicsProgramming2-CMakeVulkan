#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform sampler2D u_Textures[];

layout(push_constant) uniform Push {
    mat4 transform;     // projection * view * model
    mat4 modelMatrix;
    uint materialIndex;
} push;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(u_Textures[push.materialIndex], inUV);
}
