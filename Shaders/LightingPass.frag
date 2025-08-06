//Lighting.frag
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
//   gBuffers[3] = metalRoughness 
//   gBuffers[4] = occlusion
layout(set = 0, binding = 0) uniform sampler2D gBuffers[];

// now lights live in set 1, binding 0
struct PointLight {
    vec3 position;
    vec3 radius;
    vec3 lightColor; 
    float lightIntensity;
};
layout(set = 1, binding = 0) buffer Lights {
    PointLight lights[];
} lightsBuf;



layout(location = 0) out vec4 outColor;


vec3 lightColor = vec3(1.0,1.0,1.0); 
float ambientStrength = 0.1; 
vec3 lightDirection = vec3(0.577,-0.577,-0.577); 
float lightIntensity = 1.0;  

void main() {
    ivec2 pix = ivec2(gl_FragCoord.xy);

    vec3 pos        = texelFetch(gBuffers[0], pix, 0).xyz;
    vec3 normal     = texelFetch(gBuffers[1], pix, 0).rgb;
    vec3 albedo     = texelFetch(gBuffers[2], pix, 0).rgb;
    vec2 metalRough = texelFetch(gBuffers[3],pix, 0).rg;


    // 2) Compute ambient
    vec3 ambient = ambientStrength * lightColor;

    // 3) Diffuse (Lambert)
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor * lightIntensity;

    vec3 color = (ambient + diffuse) * albedo;
    

    outColor = vec4(metalRough.g, metalRough.g, metalRough.g, 1);
}
