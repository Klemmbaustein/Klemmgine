#version 330 

uniform sampler2D u_texture;
uniform bool u_usetexture = false;
in vec2 g_tex_coord;
void main()
{
	if(u_usetexture)
		if(texture(u_texture, g_tex_coord).a < 0.33f)
		discard;
}