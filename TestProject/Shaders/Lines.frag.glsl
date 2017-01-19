#version 120

varying float FragTime;

uniform float Time;
uniform vec3 Color;
uniform float TimeRange;

void main()
{
	gl_FragColor.a = 1;
	gl_FragColor.rgb = Color;
	if(Time < FragTime)
	{
		gl_FragColor.rgba = vec4(0,0,0,0);
	}
	else
	{
		gl_FragColor.rgb = mix(Color, vec3(0,0,0), (Time - FragTime)/TimeRange);
	}
}