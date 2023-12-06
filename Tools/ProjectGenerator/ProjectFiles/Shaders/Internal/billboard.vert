#version 330

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_tex_coords;

out vec2 v_tex_coords;

uniform mat4 u_model;
uniform mat4 u_viewpro;
uniform float u_scale;

void main()
{
	gl_Position = u_viewpro * u_model * vec4(a_position * u_scale, 1.0f);
	v_tex_coords = a_tex_coords;
}