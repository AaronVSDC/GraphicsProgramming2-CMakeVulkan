#version 450

// must match your ResolutionCameraPush in C++
layout(push_constant) uniform LightPC {
    vec2 resolution;
    float _pad0[2];
    vec3 cameraPos;
    float _pad1;
} pc;

// bindless array of your three G-Buffer attachments:
//   gBuffers[0] = world‐space position (RGBA16F)
//   gBuffers[1] = normal           (RGBA16F)
//   gBuffers[2] = albedo           (RGBA8)
layout(set = 0, binding = 0) uniform sampler2D gBuffers[];

layout(location = 0) out vec4 outColor;

void main() {
    // integer pixel coordinate
    ivec2 pix = ivec2(gl_FragCoord.xy);

    // fetch directly—no interpolation, so no half-texel shift artifacts
    vec3 pos    = texelFetch(gBuffers[0], pix, 0).xyz;
    vec3 norm   = normalize(texelFetch(gBuffers[1], pix, 0).xyz);
    vec3 alb    = texelFetch(gBuffers[2], pix, 0).rgb;

    // simple ambient + diffuse from camera location
    vec3 L     = normalize(pc.cameraPos - pos);
    float dif  = max(dot(norm, L), 0.0);
    vec3 color = 0.1 * alb + dif * alb;

    outColor = vec4(color, 1.0);
}
