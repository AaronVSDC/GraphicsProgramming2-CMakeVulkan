#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform sampler2D bindlessTextures[];
layout(location = 0) in vec2 vUV;

layout(push_constant) uniform PC {
    mat4 mvp;
    uint  albedoIndex;
} pc;

const float alphaThreshold = 0.95;

void main() {
    uint mi = pc.albedoIndex;
    if (mi != 0xFFFFFFFFu) {
        float alpha = texture(bindlessTextures[ nonuniformEXT(mi) ], vUV).a;
        if (alpha < alphaThreshold) {
            discard;
        }
    }
    
}
