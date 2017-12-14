#version 430 core

in VS_OUT
{
	vec4 Color;
	vec2 uvCoord;
} fs_in;

out vec4 Color;
layout(location = 2) uniform sampler2D TextureSampler;

void main(void)
{
	Color = fs_in.Color;
	Color = texture2D( TextureSampler, fs_in.uvCoord )
}