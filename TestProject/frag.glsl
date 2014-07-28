#version 430 core

layout (location = 0) out vec4 color;

in VS_OUT
{
	vec4 inColor;
} fs_in;

void main(void)
{
	color = fs_in.inColor;
}