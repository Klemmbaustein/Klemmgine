#version 330 

uniform sampler2D u_texture;
uniform bool u_useTexture = false;
in vec2 g_tex_coord;
void main()
{
	if(u_useTexture)
		if(texture(u_texture, g_tex_coord).a < 0.33f)
		discard;
}