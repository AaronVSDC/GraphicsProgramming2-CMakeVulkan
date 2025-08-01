#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform Push {
    mat4 transform;
    mat4 modelMatrix;
    uint materialIndex;
} push;

void main() {
    gl_Position = push.transform * vec4(inPosition, 1.0);
}