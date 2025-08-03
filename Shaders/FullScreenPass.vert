#version 450

layout(location = 0) out vec2 fsUV;

void main()
 { 
    // we hardcode a giant triangle that covers the screen:
    const vec2 verts[3] = vec2[](
        vec2(-1.0, -1.0),   // bottom-left
        vec2( 3.0, -1.0),   // bottom-right *and* off-screen
        vec2(-1.0,  3.0)    // top-left   *and* off-screen
    );

    vec2 pos = verts[gl_VertexIndex];
    gl_Position = vec4(pos, 0.0, 1.0);

    // uv mapping from -1..+1 → 0..1:
    fsUV = pos * 0.5 + 0.5;
}