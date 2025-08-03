#version 450
#extension GL_EXT_nonuniform_qualifier : require

// match the locations you wrote out from the vertex shader:
layout(location = 0) in vec3 fsWorldPos;
layout(location = 1) in vec3 fsColor;
layout(location = 2) in vec3 fsNormal;
layout(location = 3) in vec2 fsUV;
layout(location = 4) flat in uint fsMaterialIndex;

// bindless descriptor set 0, binding 0 is your array of samplers:
layout(set = 0, binding = 0) uniform sampler2D uTextures[];

layout(location = 0) out vec4 outPosition;   // R16G16B16A16
layout(location = 1) out vec4 outNormal;     // R16G16B16A16 
layout(location = 2) out vec4 outAlbedoSpec; // R8G8B8A8

void main() {
    // world‐space
    outPosition = vec4(fsWorldPos, 1.0);

    // pack normal + 1.0 into alpha (or just write 1.0):
    outNormal = vec4(normalize(fsNormal), 1.0);

    // sample the diffuse texture for this material:
    vec4 albedo = texture(uTextures[fsMaterialIndex], fsUV);
    // store RGB = albedo.rgb, A = specular power (here just 1)
    outAlbedoSpec = vec4(albedo.rgb, 1.0);
}