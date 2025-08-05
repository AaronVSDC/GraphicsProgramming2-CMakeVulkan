#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBiTangent;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec2 outUV;
layout(location = 4) out vec3 outTangent;
layout(location = 5) out vec3 outBiTangent;


void main()
 { 
    outPosition = inPosition; 
    outColor = inColor; 
    outNormal = inNormal; 
    outUV = inUV; 
    outTangent = inTangent; 
    outBiTangent = inBiTangent; 



    // use the "triangle trick"
    const vec2 verts[3] = vec2[](
    vec2(-1.0, -1.0),   
    vec2(-1.0,  3.0),   
    vec2( 3.0, -1.0)    
    );

    vec2 pos = verts[gl_VertexIndex];
    gl_Position = vec4(pos, 0.0, 1.0);

}