#version 430 core

layout(location = 0) in vec3 _Position;
layout(location = 1) in vec4 _Color;
layout(location = 2) in vec2 _UV;

out VS_OUT
{
	vec4 Color;
	vec2 uvCoord;
} vs_out;

void main(void)
{
	gl_Position = _Position;
	vs_out.Color = _Color;
	vs_out.uvCoord = _UV;
}