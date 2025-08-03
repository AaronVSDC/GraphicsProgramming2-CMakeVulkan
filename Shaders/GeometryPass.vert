#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;
// ← no more 'location=4' here

layout(push_constant) uniform PushConstants {
    mat4 transform;
    mat4 modelMatrix;
    uint materialIndex;
} pc;

layout(location = 0) out vec3  fsWorldPos;
layout(location = 1) out vec3  fsColor;
layout(location = 2) out vec3  fsNormal;
layout(location = 3) out vec2  fsUV;
layout(location = 4) flat out uint fsMaterialIndex;  // you can still write it here

void main() {
    vec4 worldPos   = pc.modelMatrix * vec4(inPosition, 1.0);
    fsWorldPos      = worldPos.xyz;
    fsNormal        = mat3(pc.modelMatrix) * inNormal;
    fsUV            = inUV;
    fsColor         = inColor;
    fsMaterialIndex = pc.materialIndex;  // ← from push constant now

    gl_Position = pc.transform * vec4(inPosition, 1.0);
}
