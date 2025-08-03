#version 450

layout(location = 0) in vec2 fsUV;

// descriptor set 0, binding 0 is a 3-element array of samplers:
layout(set = 0, binding = 0) uniform sampler2D uGBuffer[3];

// push constant: camera position in world-space
layout(push_constant) uniform CamPos { vec3 cameraPos; } pc;

// final output:
layout(location = 0) out vec4 outColor;

void main() {
    // fetch G-Buffer:
    // vec3  pos    = texture(uGBuffer[0], fsUV).xyz;
    // vec3  norm   = normalize(texture(uGBuffer[1], fsUV).xyz);
    vec3  albedo = texture(uGBuffer[2], fsUV).rgb;


    // // simple single‐point light:
    // vec3 lightPos   = vec3( 10.0, 10.0, 10.0 );
    // vec3 lightColor = vec3( 1.0 );

    // vec3 L = normalize(lightPos - pos);
    // float diff = max(dot(norm, L), 0.0);

    // vec3 V = normalize(pc.cameraPos - pos);
    // vec3 R = reflect(-L, norm);
    // float spec = pow(max(dot(V, R), 0.0), 16.0);

    // vec3 lit = (albedo * diff + spec) * lightColor;

    outColor = vec4(albedo, 1.0);
}