#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;   // unused but consumed
layout(location = 2) in vec3 inNormal;  // unused but consumed
layout(location = 3) in vec2 inUV;      // unused but consumed

layout(push_constant) uniform Push {
    mat4 transform;
} push;

void main() {
    gl_Position = push.transform * vec4(inPosition, 1.0);
}