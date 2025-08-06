//Blit.frag
#version 450
// #extension GL_GOOGLE_include_directive : enable
// #include "tm_helpers.glsl"

// layout(push_constant) uniform ToneMappingUniforms {
//     float aperture;
//     float shutterSpeed;
//     float iso;
// } tmUbo;

layout(location = 0) in vec2 fragUV;
layout(set = 0, binding = 0) uniform sampler2D litSampler;

layout(location = 0) out vec4 outColor;

void main() {
    ivec2 pix = ivec2(gl_FragCoord.xy); 

    vec3 hdr = texelFetch(litSampler, pix, 0).rgb;

    // float ev100     = CalculateEV100FromPhysicalCamera(tmUbo.aperture, tmUbo.shutterSpeed, tmUbo.iso);
    // float exposure  = ConvertEV100ToExposure(ev100);
    // vec3 mapped     = Uncharted2ToneMapping(hdr * exposure);
    // mapped          = pow(mapped, vec3(1.0 / GAMMA));

    outColor = vec4(hdr, 1.0);
}