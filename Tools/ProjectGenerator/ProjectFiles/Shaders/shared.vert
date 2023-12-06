#version 330

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_tex_coord;
layout(location = 2) in vec3 a_color;
layout(location = 3) in vec3 a_normal;
layout(location = 4) in mat4 a_model;
out vec3 v_position;
out vec3 v_untransformedposition;
out vec2 v_texcoord;
out vec3 v_fragposlightspace;
out vec3 v_screenposition;
out vec3 v_normal;
out vec3 v_screennormal;
uniform mat4 u_modelviewpro;
uniform mat4 u_invmodelview;
uniform mat4 u_lightspacematrix;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_viewpro;
uniform vec3 u_cameraposition = vec3(0);

struct DirectionalLight
{
	vec3 Direction;
	vec3 SunColor;
	vec3 AmbientColor;
	float Intensity;
	float AmbientIntensity;
};
uniform DirectionalLight u_directionallight;

vec3 TranslatePosition(vec3 relativePos)
{
	return vec3(a_model * vec4(relativePos, 1.0));
}

void ReturnPositionNormal(vec3 pos, vec3 normal)
{
	v_position = pos;

	gl_Position = (u_viewpro) * vec4(v_position, 1);
	v_fragposlightspace = (u_lightspacematrix * vec4(a_position, 1.f)).rgb;
	v_normal = mat3(a_model) * normal;
	v_screennormal = normalize(transpose(inverse((u_view))) * vec4(v_normal, 1)).xyz;
	v_screenposition = (u_view * vec4(v_position, 1)).rgb;
	v_texcoord = a_tex_coord;
}

void ReturnPosition(vec3 pos)
{
	ReturnPositionNormal(pos, a_normal);
}