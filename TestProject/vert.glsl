#version 430 core

//layout(std140, binding = 0) uniform MatrixTransformations
//{						//base alignment			offset			aligned offset
//	mat4 m;				//16						0					0
//	mat4 v;				//16						64					64
//	mat4 p;				//16						128					128
//	mat4 mv;			//16						192					192
//	mat4 mvp;			//16						256					256
//};

layout(location = 0) uniform mat4 mvp;

out VS_OUT
{
	vec4 inColor;
} vs_out;


layout(location = 0) in vec4 position;

void main(void)
{
	vs_out.inColor = vec4(1.0, 0.0, 1.0, 1.0);

	gl_Position = position * mvp;
}