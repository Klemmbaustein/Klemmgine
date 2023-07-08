//! #include "shared.frag"
uniform vec3 u_skycolor = vec3(0.3f, 0.6f, 1.f);
uniform vec3 u_horizoncolor = vec3(0.7, 0.8, 1);

void main()
{
	vec3 normal = normalize(-v_position);
	vec3 color = mix(u_horizoncolor, u_skycolor, abs(dot(vec3(normalize(normal)), vec3(0, 1, 0))));
	if (max(pow(dot(normalize(-normal), normalize(u_directionallight.Direction)), 100), 0) > 1 - min(u_directionallight.Intensity / 10, 0.9))
	{
		color = mix(color, u_directionallight.SunColor, u_directionallight.Intensity) * 1.5;
	}
	RETURN_COLOR_NO_FOG(color)
}