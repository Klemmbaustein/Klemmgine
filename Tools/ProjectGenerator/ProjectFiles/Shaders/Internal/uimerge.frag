#version 330
uniform sampler2D u_texture;
uniform sampler2D u_alpha;
in vec2 v_texcoords;

out vec4 f_color;


void main()
{
	vec4 color = texture(u_texture, v_texcoords);
	vec4 alpha = texture(u_alpha, v_texcoords);
	f_color = vec4(color.xyz / alpha.r, alpha.r);
}