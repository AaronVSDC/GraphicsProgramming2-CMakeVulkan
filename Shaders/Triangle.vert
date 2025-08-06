//triangle.vert
#version 450

layout(location = 0) out vec2 fragUV;

void main()
 { 

    // use the "triangle trick"
    const vec2 verts[3] = vec2[](
    vec2(-1.0, -1.0),   
    vec2(-1.0,  3.0),   
    vec2( 3.0, -1.0)    
    );

    vec2 pos = verts[gl_VertexIndex];
    fragUV = pos * 0.5 + 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);

}