#version 120

varying float FragTime;

uniform float Time;
uniform vec3 Color;
uniform float TimeRange;

void main()
{
	gl_FragColor.rgb = Color;
	if(Time < FragTime)
	{
		gl_FragColor.a = 0;
	}
	else
	{
		gl_FragColor.a = mix(1, 0, (Time - FragTime)/TimeRange);
	}
}