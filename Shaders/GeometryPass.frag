#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(early_fragment_tests) in;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragUV; //I dont think we actually need them over here. 
layout(location = 4) in vec3 fragTangent;
layout(location = 5) in vec3 fragBiTangent;


layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormalMap;
layout(location = 2) out vec4 outAlbedoMap;
layout(location = 3) out vec4 outMetalRoughnessMap;
layout(location = 4) out vec4 outOcclusionMap;

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
        mr = texture(bindlessTextures[ nonuniformEXT(pc.metalRoughIndex) ], fragUV).bg;
        }

    float occ = 1.0;
    if (pc.occlusionIndex != 0xFFFFFFFFu) {
        occ = texture(bindlessTextures[ nonuniformEXT(pc.occlusionIndex) ], fragUV).r;
    }

    vec3 normal = vec3(0.0); 
    if(pc.normalIndex != 0xFFFFFFFFu) {
        mat3 TBN = mat3(
        normalize(fragTangent),
        normalize(fragBiTangent),
        normalize(fragNormal)
        );

        vec3 sampledNormal = texture(bindlessTextures[nonuniformEXT(pc.normalIndex)], fragUV).rgb * 2.0 - 1.0;        
        normal = normalize(TBN * sampledNormal); 
     }
 
    // output G-Buffer
    outPosition          = vec4(fragPos,   1.0);
    outNormalMap         = vec4(normal * 0.5 + 0.5, 1.0); 
    outAlbedoMap         = vec4(albedo,  1.0);
    outMetalRoughnessMap = vec4(mr, 0.0, 1.0); 
    outOcclusionMap      = vec4(occ, 0.0, 0.0, 1.0);
}
