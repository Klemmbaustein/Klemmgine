//! #include "shared.frag"

// #params Texture
// Diffuse texture
uniform sampler2D u_texture;
// 'true' if using a diffuse texture
uniform bool u_useTexture = false;

// #params Phong
// Diffuse color
uniform vec3 u_diffuse = vec3(1);
// Specular highlight size
uniform float u_specularSize = 0;
// Specular highlight intensity
uniform float u_specularStrength = 0;
// Emissive color
uniform vec3 u_emissive = vec3(0, 0, 0);
// Should the rear side of a two-sided face have it's normal reversed
uniform bool u_reverseNormal = true;

uniform float u_time = 0;

void main()
{
	bool transparent = false;
	vec4 texcolor;
	if(u_useTexture)
	{
		texcolor = vec4(u_diffuse, 1.0) * texture(u_texture, v_texcoord);
		transparent = texcolor.w < 0.33f ? true : false;
	}
	else
		texcolor = vec4(u_diffuse, 1);
	if(transparent)
		discard;

	invertReverseFaceNormal = u_reverseNormal;
	vec3 color = GetLighting(texcolor.xyz, u_specularStrength, u_specularSize) + u_emissive;
	RETURN_COLOR(color);
}