//! #include "../shared.frag"

// #params
uniform sampler2D u_texture;

void main()
{
	vec4 tex = texture(u_texture, v_texcoord);
	if (tex.w < 0.5) discard;
	RETURN_COLOR_NO_FOG(pow(tex.xyz, vec3(0.6)) * 0.875);
}