#version 450
#extension GL_EXT_nonuniform_qualifier : require
 
layout(set = 0, binding = 0) uniform sampler2D u_Textures[];

layout(push_constant) uniform Push {
    mat4 transform;
    mat4 modelMatrix;
    uint materialIndex;
} push;

// **READ BOTH varyings** at the same locations you wrote them:
layout(location = 0) in  vec3  litColor;
layout(location = 1) in  vec2  fragUV;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 albedo = texture(u_Textures[push.materialIndex], fragUV);
    outColor = vec4(litColor, 1.0) * albedo;
}
