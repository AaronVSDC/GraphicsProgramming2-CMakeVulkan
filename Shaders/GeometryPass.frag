#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(early_fragment_tests) in;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragUV;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;

layout(set = 0, binding = 0) uniform sampler2D bindlessTextures[];

layout(push_constant) uniform PC {
    mat4 transform;
    mat4 modelMatrix;
    uint diffuseIndex;
    uint maskIndex;
} pc;

const float alphaThreshold = 0.95;

void main() {
    uint mi = pc.maskIndex;
    if (mi != 0xFFFFFFFFu) {
        float mask = texture(
            bindlessTextures[ nonuniformEXT(mi) ],
            fragUV
        ).r; //TODO: looks ugly now to sample the red channel, you can fix it by actually loading in the black and white image as a grayscale instead of an rgba picture. But this approach "works"
        if (mask < alphaThreshold) {
            discard;
        }
    }

    // fetch diffuse
    vec3 albedo = texture(
        nonuniformEXT(bindlessTextures[ nonuniformEXT(pc.diffuseIndex) ]),
        fragUV
    ).rgb * fragColor;

    // output G-Buffer
    outPosition   = vec4(fragPos,   1.0);
    outNormal     = vec4(normalize(fragNorm), 1.0);
    outAlbedo     = vec4(albedo,  1.0); 
}
