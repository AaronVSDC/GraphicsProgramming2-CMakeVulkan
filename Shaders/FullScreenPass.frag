#version 450

layout(set = 0, binding = 0) uniform sampler2D gAlbedo;
layout(set = 0, binding = 1) uniform sampler2D gNormal;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));
const float AMBIENT = 0.45;

void main() {
    vec3 albedo = texture(gAlbedo, fragUV).rgb;
    vec3 normal = texture(gNormal, fragUV).xyz * 2.0 - 1.0;
    float light = AMBIENT + max(dot(normal, DIRECTION_TO_LIGHT), 0.0);
    outColor = vec4(albedo * light, 1.0);
}