#version 120

varying float Depth;

uniform vec3 Color;

void main()
{
	gl_FragColor = mix(vec4(Color, 1), vec4(0,0,0,0), -Depth);
}