//! #include "shared.frag"
uniform vec3 u_skycolor = vec3(0.3f, 0.6f, 1.f);
uniform vec3 u_horizoncolor = vec3(0.7, 0.8, 1);

void main()
{
	vec3 normal = normalize(-v_position);
	vec3 color = mix(u_horizoncolor, u_skycolor, abs(dot(vec3(normalize(normal)), vec3(0, 1, 0))));
	if (max(pow(dot(normalize(-normal), normalize(u_directionallight.Direction)) - 0.999, 1.1) * 25, 0) > 0.0)
	{
		color = u_directionallight.SunColor * 2;
	}
	RETURN_COLOR_NO_FOG(color)
}