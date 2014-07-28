#version 430 core //FRAGMENT STANDARD

layout(std140, binding = 0) uniform MatrixTransformations
{						//base alignment			offset			aligned offset
	mat4 M;				//16						0					0
	mat4 V;				//16						64					64
	mat4 P;				//16						128					128
	mat4 MV;			//16						192					192
	mat4 MVP;			//16						256					256
};

out vec4 _Color;

layout(location = 5) uniform sampler2D TextureSampler;

in VS_OUT
{
	vec2 uvCoord;
} fs_in;

