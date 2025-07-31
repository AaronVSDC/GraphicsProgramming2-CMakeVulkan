#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 litColor;   // was your fragColor
layout(location = 1) out vec2 fragUV;     // new: carry UV

layout(push_constant) uniform Push {
    mat4 transform;     // projection * view * model
    mat4 modelMatrix;
} push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));
const float AMBIENT = 0.45;

void main() {
    // standard transform
    gl_Position = push.transform * vec4(inPosition, 1.0);

    // compute world-space normal
    mat3 normalMatrix = transpose(inverse(mat3(push.modelMatrix)));
    vec3 normalWS = normalize(normalMatrix * inNormal);

    // simple ambient+diffuse
    float intensity = AMBIENT + max(dot(normalWS, DIRECTION_TO_LIGHT), 0.0);

    // emit lit color
    litColor = intensity * inColor;

    // pass through UV
    fragUV = inUV;
}
