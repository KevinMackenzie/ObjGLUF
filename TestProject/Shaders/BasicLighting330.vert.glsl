#version 430 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 _Position;
layout(location = 1) in vec3 _Normal;
layout(location = 2) in vec2 _UV;


// Output data ; will be interpolated for each fragment.
//out vec2 UV;
out VS_OUT
{
	vec2 uvCoord;
} vs_out;

out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;
out float Distance;

// Values that stay constant for the whole mesh.
//layout(std140, binding = 0) uniform MatrixTransformations
//{
//	mat4 M;
//	mat4 V;
//	mat4 P;
//	mat4 MV;
//	mat4 MVP;
//};

layout(location = 0) uniform mat4 M;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 MVP;

layout(location = 3) uniform vec3 LightPosition_worldspace;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(_Position,1);
	
	// Position of the vertex, in worldspace : M * position
	Position_worldspace = (M * vec4(_Position,1)).xyz;
	
	// Vector that goes from the vertex to the camera, in camera space.
	// In camera space, the camera is at the origin (0,0,0).
	vec3 vertexPosition_cameraspace = ( V * M * vec4(_Position,1)).xyz;
	EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	vec3 LightPosition_cameraspace = ( V * vec4(LightPosition_worldspace,1)).xyz;
	LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
	
	// Normal of the the vertex, in camera space
	Normal_cameraspace = ( V * M * vec4(_Normal,0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
	
	// UV of the vertex. No special space for this one.
	vs_out.uvCoord = _UV;
	
	// Distance to the light
	Distance = length( LightPosition_worldspace - Position_worldspace );
}

