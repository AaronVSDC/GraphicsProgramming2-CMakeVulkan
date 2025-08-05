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
layout(location = 3) out vec4 outMetalRough;
layout(location = 4) out vec4 outOcclusion;

layout(set = 0, binding = 0) uniform sampler2D bindlessTextures[];

layout(push_constant) uniform PC {
    mat4 transform;
    mat4 modelMatrix;
    uint albedoIndex; 
    uint normalIndex;
    uint metalRoughIndex;
    uint occlusionIndex;
} pc;

const float alphaThreshold = 0.95;

void main() {
 vec4 base = vec4(fragColor, 1.0);
    if (pc.albedoIndex != 0xFFFFFFFFu) {
        base *= texture(bindlessTextures[ nonuniformEXT(pc.albedoIndex) ], fragUV);
    }
    if (base.a < alphaThreshold) {
        discard;
    }
    vec3 albedo = base.rgb;

    vec2 mr = vec2(0.0, 1.0);
    if (pc.metalRoughIndex != 0xFFFFFFFFu) {
        mr = texture(bindlessTextures[ nonuniformEXT(pc.metalRoughIndex) ], fragUV).rg;
    }

    float occ = 1.0;
    if (pc.occlusionIndex != 0xFFFFFFFFu) {
        occ = texture(bindlessTextures[ nonuniformEXT(pc.occlusionIndex) ], fragUV).r;
    }

    // output G-Buffer
    outPosition   = vec4(fragPos,   1.0);
    outNormal     = vec4(normalize(fragNorm), 1.0);
    outAlbedo     = vec4(albedo,  1.0);
    outMetalRough = vec4(mr, 0.0, 1.0); 
    outOcclusion  = vec4(occ, 0.0, 0.0, 1.0);
}
