//triangle.vert
#version 450


void main()
 { 

    // use the "triangle trick"
    const vec2 verts[3] = vec2[](
    vec2(-1.0, -1.0),   
    vec2(-1.0,  3.0),   
    vec2( 3.0, -1.0)    
    );

    vec2 pos = verts[gl_VertexIndex];
    gl_Position = vec4(pos, 0.0, 1.0);

}