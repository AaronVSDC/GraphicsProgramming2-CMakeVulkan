//Lighting.frag
#version 450
#extension GL_GOOGLE_include_directive : enable
#include "LightingHelpers.glsl"


// must match your ResolutionCameraPush in C++
layout(push_constant) uniform LightPC {
    mat4 view;
    mat4 proj;  
    vec2 viewportSize;
    float _pad0[2];
    vec3 cameraPos;
    uint lightCount;
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
struct Light {
    vec3 position;
    float radius;
    vec3 direction;
    uint type; // 0 = point, 1 = directional
    vec3 lightColor;
    float lightIntensity;
};
layout(set = 1, binding = 0) readonly buffer Lights {
    Light lights[];
} LightsData;

layout(binding = 6) uniform samplerCube environmentMap; 
layout(binding = 7) uniform samplerCube irradianceMap; 


layout(location = 0) out vec4 outColor;
const float MIN_ROUGHNESS = 0.045;


const uint LIGHT_TYPE_POINT = 0;
const uint LIGHT_TYPE_DIRECTIONAL = 1;



void main() {
    ivec2 pix = ivec2(gl_FragCoord.xy);

    vec3 worldPosSample   = texelFetch(gBuffers[0], pix, 0).xyz;
    vec3 normalSample     = texelFetch(gBuffers[1], pix, 0).rgb;
    normalSample = normalize(normalSample * 2.0 - 1.0);

    vec3 albedoSample     = texelFetch(gBuffers[2], pix, 0).rgb;
    vec3 metalRoughSample = texelFetch(gBuffers[3],pix, 0).rgb;
    float depthSample     = texelFetch(gDepth, pix, 0).r;

    float metallic = metalRoughSample.r; 
    float roughness = max(metalRoughSample.g, MIN_ROUGHNESS); 

    // 0. Depth check for skybox
    if (depthSample >= 1.0) 
    {
        vec2 fragCoord = vec2(gl_FragCoord.xy);
        vec3 viewDir = normalize(GetWorldPositionFromDepth(depthSample, fragCoord, pc.viewportSize, inverse(pc.proj), inverse(pc.view)));
        outColor = vec4(texture(environmentMap, viewDir).rgb, 1.0);
        return;
     }

    vec3 litColor = vec3(0.0); 


    for(int i = 0; i < pc.lightCount; ++i)
    {
        Light light = LightsData.lights[i]; 

        if(light.type == LIGHT_TYPE_POINT)
        {
            vec3 L = light.position - worldPosSample; 
            float distance = length(L); 
            if(distance < light.radius)
            {
                float attenuation = 1.0 / (distance * distance + 0.0001);
                litColor += CalculatePBR_Point(albedoSample, normalSample, metallic, roughness, worldPosSample, light.position, light.lightColor, light.lightIntensity * attenuation, pc.cameraPos);
            }
        }
        else if(light.type == LIGHT_TYPE_DIRECTIONAL)
        {
            litColor += CalculatePBR_Directional(albedoSample, normalSample, metallic, roughness, worldPosSample, light.direction, light.lightColor, light.lightIntensity, pc.cameraPos);   
        }
    }
    

    vec3 iblColor = CalculateDiffuseIrradiance( irradianceMap ,albedoSample, normalSample);
    litColor += iblColor; 

    outColor = vec4(litColor, 1.0);

}
