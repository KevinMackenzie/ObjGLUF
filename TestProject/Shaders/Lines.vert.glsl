#version 120

attribute vec3 vertexPosition_modelspace;

varying float FragTime;

uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4(vertexPosition_modelspace.yz, -1, 1);
	FragTime = vertexPosition_modelspace.x;
}