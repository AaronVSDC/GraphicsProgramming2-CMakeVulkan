//Lighting.frag
#version 450
#extension GL_GOOGLE_include_directive : enable
#include "LightingHelpers.glsl"


// must match your ResolutionCameraPush in C++
layout(push_constant) uniform LightPC {
    vec2 viewportSize;
    float _pad0[2];
    vec3 cameraPos;
    float _pad1;
} pc;

// bindless array of your three G-Buffer attachments:
//   gBuffers[0] = world‐space position (RGBA16F)
//   gBuffers[1] = normal           (RGBA16F)
//   gBuffers[2] = albedo           (RGBA8)
//   gBuffers[3] = metalRoughness 
//   gBuffers[4] = occlusion
layout(set = 0, binding = 0) uniform sampler2D gBuffers[];
layout(set = 0, binding = 5) uniform sampler2D gDepth;

// now lights live in set 1, binding 0
struct PointLight {
    vec3 position;
    float radius;
    vec3 lightColor; 
    float lightIntensity;
};
layout(set = 1, binding = 0) readonly buffer Lights {
    PointLight lights[];
} PointLights;



layout(location = 0) out vec4 outColor;
const float MIN_ROUGHNESS = 0.045;


//HELPERS-------------------------------------------------



void main() {
    ivec2 pix = ivec2(gl_FragCoord.xy);

    vec3 worldPosSample   = texelFetch(gBuffers[0], pix, 0).xyz;
    vec3 normalSample     = texelFetch(gBuffers[1], pix, 0).rgb;
    vec3 albedoSample     = texelFetch(gBuffers[2], pix, 0).rgb;
    vec2 metalRoughSample = texelFetch(gBuffers[3],pix, 0).rg;
    float depthSample     = texelFetch(gDepth, pix, 0).r;

    float metallic = metalRoughSample.r; 
    float roughness = max(metalRoughSample.g, MIN_ROUGHNESS); 

    vec3 litColor = vec3(0.0); 

    uint pointLightCount = 1; //todo: make sure you actually keep the amount of lights in the scene.  

    for(int i = 0; i < pointLightCount; ++i)
    {
        PointLight pl = PointLights.lights[i]; 
        vec3 L = pl.position - worldPosSample; 
        float distance = length(L);

        if(distance < pl.radius)
        {
            float attenuation = 1.0 / (distance * distance + 0.0001);

            litColor += CalculatePBR_Point(albedoSample, normalSample, metallic, roughness, worldPosSample, pl.position, pl.lightColor, pl.lightIntensity * attenuation, pc.cameraPos); 

        }

    }
    
    outColor = vec4(litColor, 1.0);

}
