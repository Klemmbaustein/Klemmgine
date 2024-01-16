#version 330
layout(location = 4) in mat4 a_model;
layout(location = 0) in vec3 a_position;
uniform mat4 u_viewpro;

void main()
{
	gl_Position = u_viewpro * vec4(a_model * vec4(a_position.xyz, 1));
}