#version 450

// incoming from vertex
layout(location = 0) in vec3 litColor;
layout(location = 1) in vec2 fragUV;

// our texture binding (set 0, binding 0)
layout(set = 0, binding = 0) uniform sampler2D uTexture;

// final color output
layout(location = 0) out vec4 outColor;

void main() {
    // fetch texture
    vec4 tex = texture(uTexture, fragUV);

    // combine texture rgb with your lit color, preserve texture alpha
    outColor = vec4(litColor * tex.rgb, tex.a);
}
