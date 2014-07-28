#version 430 core //VERTEX STANDARD

layout(std140, binding = 0) uniform MatrixTransformations
{						//base alignment			offset			aligned offset
	mat4 M;				//16						0					0
	mat4 V;				//16						64					64
	mat4 P;				//16						128					128
	mat4 MV;			//16						192					192
	mat4 MVP;			//16						256					256
};

layout(location = 0) in vec3 _Position;
layout(location = 1) in vec3 _Normal;
layout(location = 2) in vec2 _UV;

out VS_OUT
{
	vec2 uvCoord;
} vs_out;

out gl_PerVertex
{
    vec4 gl_Position;
};
