#version 460 core
layout(location = 0) in  vec4 PosTex;
layout(location = 0) out vec2 TexCoords;


uniform mat4 modelMatrix;
uniform mat4 view;
uniform mat4 projection;


void main()
{
	vec2 vertexPosition = PosTex.xy;
	vec4 vpos4 = vec4(vertexPosition, 0.0f, 1.0f);

	vpos4 = projection * view * modelMatrix * vpos4;

	gl_Position = vpos4;
	TexCoords   = PosTex.zw;
}