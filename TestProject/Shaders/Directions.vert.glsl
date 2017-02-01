#version 330 core

in vec3 vertexPosition_modelspace;

out float Direction;

void main()
{
	vec4 pos = vec4(vertexPosition_modelspace,1);
	Direction = pos.z;
	pos.z = 0;
	gl_Position = pos;
}