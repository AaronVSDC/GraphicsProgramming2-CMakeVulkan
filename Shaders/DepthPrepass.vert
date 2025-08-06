//DepthPrepass.vert
#version 450

layout(push_constant) uniform PC {
    mat4 mvp;       // projection * view * model
    uint  maskIndex;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 3) in vec2 inUV;             // we just need UV for mask test

layout(location = 0) out vec2 vUV;

void main() {
    gl_Position = pc.mvp * vec4(inPosition, 1.0);
    vUV         = inUV;
}
