//! #include "shared.vert"

struct DirectionalLight
{
	vec3 Direction;
	vec3 SunColor;
	vec3 AmbientColor;
	float Intensity;
	float AmbientIntensity;
};

uniform DirectionalLight u_directionallight;

void main()
{

	ReturnPosition(TranslatePosition(a_position));
	if (dot(v_normal, u_directionallight.Direction) < 0)
	{
		v_modelnormal = -v_modelnormal; //Faked Subsurface Scattering
		v_normal = -v_normal;
		v_screennormal = v_screennormal;
	}
}