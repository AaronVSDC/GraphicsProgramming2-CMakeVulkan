#version 450

layout(location = 0) in vec2 fsUV;

// descriptor set 0, binding 0 is a 3-element array of samplers:
layout(set = 0, binding = 0) uniform sampler2D uGBuffer[3];

// push constant: camera position in world-space
layout(push_constant) uniform PC {
    vec2 resolution;   // (width, height)
    vec3 cameraPos;    // your existing camera position
} pc;
// final output:
layout(location = 0) out vec4 outColor;

void main() 
{

    ivec2 pix = ivec2(gl_FragCoord.xy);
    // fetch G-Buffer:
    vec3  pos    = texelFetch(uGBuffer[0], pix, 0).xyz; 
    vec3  norm   = normalize(texelFetch(uGBuffer[1], pix, 0).xyz);
    vec3  albedo = texelFetch(uGBuffer[2], pix, 0).rgb;


    // // simple single‐point light:
    vec3 lightPos   = vec3( 10.0, 10.0, 10.0 );
    vec3 lightColor = vec3( 1.0 ); 

    vec3 L = normalize(lightPos - pos);
    float diff = max(dot(norm, L), 0.0);

    vec3 V = normalize(pc.cameraPos - pos);
    vec3 R = reflect(-L, norm);
    vec3 lit = (albedo * diff) * lightColor;

    outColor = vec4(lit, 1.0);
}