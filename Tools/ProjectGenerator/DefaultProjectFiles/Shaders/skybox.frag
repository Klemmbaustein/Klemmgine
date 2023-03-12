//! #include "shared.frag"
uniform vec3 u_skycolor = vec3(0.3f, 0.6f, 1.f);
uniform vec3 u_horizoncolor = vec3(0.7, 0.8, 1);

void main()
{
	vec3 normal = normalize(-v_position);
	vec3 color = mix(u_horizoncolor, u_skycolor, abs(dot(vec3(normalize(normal)), vec3(0, 1, 0))));
	vec3 SunOffset = vec3(250, 250, 250) * max(pow(dot(normalize(-normal), normalize(u_directionallight.Direction)) - 0.999, 1.1) * 25, 0);
	SunOffset = min(SunOffset, 1);
	SunOffset *= u_directionallight.SunColor * 2;
	color += SunOffset;
	RETURN_COLOR_NO_FOG(color)
}