#version 120

attribute vec3 vertexPosition_modelspace;

varying float Depth;

uniform mat4 MVP;
uniform float Time;
uniform float TimeRange;

void main()
{
	vec4 pos = vec4(vertexPosition_modelspace.yz, 0, 1);
	pos.z = Depth = 1+(vertexPosition_modelspace.x - Time)/TimeRange;
	gl_Position = MVP * pos;
}