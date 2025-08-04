#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform sampler2D bindlessTextures[];
layout(location = 0) in vec2 vUV;

layout(push_constant) uniform PC {
    mat4 mvp;
    uint  maskIndex;
} pc;

const float alphaThreshold = 0.95;

void main() {
    uint mi = pc.maskIndex;
    if (mi != 0xFFFFFFFFu) {
        float alpha = texture(bindlessTextures[ nonuniformEXT(mi) ], vUV).r; //TODO: looks ugly now to sample the red channel, you can fix it by actually loading in the black and white image as a grayscale instead of an rgba picture. But this approach "works"

        if (alpha < alphaThreshold) 
        {
            discard;
        } 
    }
    
}
