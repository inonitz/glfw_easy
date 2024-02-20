#version 460 core
layout(location = 0) in  vec3 Pos;
layout(location = 1) in  vec3 Col;
layout(location = 2) in  vec2 TexPos;

layout(location = 0) out vec3 _UseAttribute;
layout(location = 1) out vec3 _Col;
layout(location = 2) out vec2 _TexPos;


uniform uvec2 WindowSize;
uniform mat4  Projection;
uniform vec3 UseAttribute;


void main()
{
	vec2  winSize = WindowSize;
	float aspect = winSize.x / winSize.y;

	vec3 vertexPosition = Pos;
	vec4 vpos4 = vec4(vertexPosition, 1.0f);
	vpos4 = Projection * vpos4;


	gl_Position   = vpos4;
    _Col 		  = Col;
	_TexPos 	  = TexPos;

	_UseAttribute = UseAttribute;
}