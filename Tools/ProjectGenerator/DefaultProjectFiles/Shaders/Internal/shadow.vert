#version 430 
layout (location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_tex_coord;
layout(location = 4) in mat4 a_model;

out vec2 v_tex_coord;
void main()
{
	gl_Position = a_model * vec4(a_pos, 1.0);
	v_tex_coord = a_tex_coord;
}