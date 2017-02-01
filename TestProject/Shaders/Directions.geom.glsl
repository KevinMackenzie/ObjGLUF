#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 9) out;

in float[] Direction;

uniform float StemLength;
uniform float StemWidth;
uniform float HeadLength;
uniform float HeadWidth;

uniform mat4 MVP;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main()
{
	//I think there's a better way to do this
	mat4 rot = rotationMatrix(vec3(0,0,1),-Direction[0]);
	
	//Top triangle of stem
	gl_Position = gl_in[0].gl_Position + rot * vec4(0, StemWidth / 2, 0, 1);
	gl_Position = MVP * gl_Position;
	EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + rot * vec4(StemLength, StemWidth / 2, 0, 1);
	gl_Position = MVP * gl_Position;
	EmitVertex();	
	
	gl_Position = gl_in[0].gl_Position + rot * vec4(StemLength, -StemWidth / 2, 0, 1);
	gl_Position = MVP * gl_Position;
	EmitVertex();
	EndPrimitive();
	
	
	//bottom triangle of stem
	gl_Position = gl_in[0].gl_Position + rot * vec4(0, StemWidth / 2, 0, 1);
	gl_Position = MVP * gl_Position;
	EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + rot * vec4(StemLength, -StemWidth / 2, 0, 1);
	gl_Position = MVP * gl_Position;
	EmitVertex();	
	
	gl_Position = gl_in[0].gl_Position + rot * vec4(0, -StemWidth / 2, 0, 1);
	gl_Position = MVP * gl_Position;
	EmitVertex();
	EndPrimitive();
	
	//direction triangle
	gl_Position = gl_in[0].gl_Position + rot * vec4(StemLength, HeadWidth / 2, 0, 1);
	gl_Position = MVP * gl_Position;
	EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + rot * vec4(StemLength + HeadLength, 0, 0, 1);
	gl_Position = MVP * gl_Position;
	EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + rot * vec4(StemLength, -HeadWidth / 2, 0, 1);
	gl_Position = MVP * gl_Position;
	EmitVertex();
	EndPrimitive();
}