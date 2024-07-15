#version 450 core
// layout(location = 0) in  vec4 vPos;
// layout(location = 1) in  vec2 vTexCoord;
// layout(location = 0) out vec2 texCoords;
layout(location = 1) out vec4 pos;




const vec2 quadVertices[4] = 
( 
	vec2(-1.0, -1.0), 
	vec2( 1.0, -1.0), 
	vec2(-1.0,  1.0), 
	vec2( 1.0,  1.0) 
);


void main()
{
	// gl_Position = pos = vPos;
	// texCoords   = vTexCoord;
	
	gl_Position = vec4(quadVertices[gl_VertexID], 0.0, 1.0);	
	return;
}