//! #include "shared.frag"
uniform sampler2D u_texture;
uniform vec3 u_diffuse;
uniform vec3 u_emissive;
uniform int u_usetexture;
uniform float u_specularsize;
uniform float u_specularstrength;
uniform float u_time;

void main()
{
	bool transparent = false;
	vec3 texcolor;
	if(u_usetexture == 1)
	{
		texcolor = u_diffuse * texture(u_texture, v_texcoord).rgb;
		transparent = texture(u_texture, v_texcoord).w < 0.33f ? true : false;
	}
	else
		texcolor = u_diffuse;
	if(transparent)
		discard;
	vec3 color = GetLighting(texcolor, u_specularstrength, u_specularsize) + u_emissive;
	RETURN_COLOR(color);
}