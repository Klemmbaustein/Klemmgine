#version 330
uniform sampler2D u_texture;
uniform sampler2D u_alpha;
in vec2 v_texcoords;

out vec4 f_color;

void main()
{
	f_color = vec4(texture(u_texture, v_texcoords).xyz, texture(u_alpha, v_texcoords).r);
}