//! #include "shared.frag"

// #params colors
uniform vec3 u_skycolor = vec3(0.3, 0.6, 1);
uniform vec3 u_horizoncolor = vec3(0.7, 0.8, 1);

void main()
{
	vec3 normal = normalize(-v_position);
	vec3 color = mix(u_horizoncolor, u_skycolor, abs(dot(vec3(normalize(normal)), vec3(0, 1, 0))));

	float dir = max(pow(dot(normalize(-normal), normalize(u_directionallight.Direction)), 50 / (u_directionallight.Intensity * 0.5) * 10), 0);

	color = mix(color, u_directionallight.SunColor * max(u_directionallight.Intensity / 1.5, 1), dir);
	RETURN_COLOR_NO_FOG(color)
}