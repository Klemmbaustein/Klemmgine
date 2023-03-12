#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_tex_coord;
layout(location = 2) in vec3 a_color;
layout(location = 3) in vec3 a_normal;
layout(location = 4) in mat4 a_model;
out vec3 v_position;
out vec3 v_untransformedposition;
out vec2 v_texcoord;
out vec3 v_normal;
out vec3 v_fragposlightspace;
out vec3 v_screenposition;
out vec3 v_modelnormal;
out vec3 v_normmodelnormal;
out vec3 v_screennormal;
out float depth;
uniform mat4 u_modelviewpro;
uniform mat4 u_invmodelview;
uniform mat4 u_lightspacematrix;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_viewpro;
uniform vec3 u_cameraposition;
void main()
{
	gl_Position = (u_viewpro) * a_model * vec4(a_position + u_cameraposition / 50, 1.0f);
	depth = gl_Position.z;
	v_fragposlightspace = (u_lightspacematrix * vec4(a_position, 1.f)).rgb;
	v_modelnormal = (mat3(a_model) * a_normal);
	v_normal = a_normal;
	v_normmodelnormal = normalize(mat3(a_model) * a_normal);
	v_screennormal = (transpose(inverse(mat3(a_model * u_view))) * (a_normal));
	v_screenposition = (u_view * a_model * vec4(a_position, 1)).rgb;
	v_untransformedposition = a_position;
    v_position = vec3(a_model * vec4(a_position, 1.0));
	v_texcoord = a_tex_coord;
}