//Blit.frag
#version 450
#extension GL_GOOGLE_include_directive : enable
#include "ToneMappingHelpers.glsl"

#define INDOOR


#ifdef SUNNY_16 
float aperture = 5.0f;
float shutterSpeed = 1.0f/200.f;
float iso = 100.f;
#endif

#ifdef INDOOR
float aperture = 1.4f;
float shutterSpeed = 1.0f/60.f;
float iso = 1600.f;
#endif

layout(location = 0) in vec2 fragUV;
layout(set = 0, binding = 0) uniform sampler2D litSampler;

layout(location = 0) out vec4 outColor;

void main() {
    ivec2 pix = ivec2(gl_FragCoord.xy); 

    vec3 litColor = texelFetch(litSampler, pix, 0).rgb;

    float ev100     = CalculateEV100FromPhysicalCamera(aperture, shutterSpeed, iso);
    float exposure  = ConvertEV100ToExposure(ev100);
    vec3 mapped     = Uncharted2ToneMapping(litColor * exposure);
    mapped          = pow(mapped, vec3(1.0 / GAMMA)); 

    outColor = vec4(litColor, 1.0);
}