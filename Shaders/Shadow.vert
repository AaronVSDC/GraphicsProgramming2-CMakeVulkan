#version 450

layout(push_constant) uniform PushConstants {
    mat4 lightViewProj;   // exactly 64 bytes
} ps;

// Only position is needed
layout(location = 0) in vec3 inPosition;

void main() {
    // Transform into light’s clip space and let the fixed-function depth test run
    gl_Position = ps.lightViewProj * vec4(inPosition, 1.0);
}