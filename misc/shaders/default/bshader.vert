#version 460 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 vel;
layout(location = 2) in vec3 col;
layout(location = 0) out vec3 particleColor;



uniform mat2 modelmatrix;
uniform float particleSize;


void main()
{
	gl_Position   = vec4(modelmatrix * pos, 0, 1.0f);
	gl_PointSize  = particleSize; 
	particleColor = col;
}