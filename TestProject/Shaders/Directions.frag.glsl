#version 330 core

uniform vec3 Color;

void main()
{
	gl_FragColor = vec4(Color,1);
}