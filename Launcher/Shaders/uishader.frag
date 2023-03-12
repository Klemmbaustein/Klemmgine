#version 330

in vec2 v_texcoords;
in vec2 v_position;
out vec4 f_color;

uniform vec4 u_color = vec4(1);
uniform int u_usetexture;
uniform sampler2D u_texture;
uniform vec3 u_offset; // Scroll bar: X = scrolled distance; Y = MaxDistance; Z MinDistance
uniform float u_opacity = 1;
uniform int u_borderType = 1;
uniform float u_borderScale = 0.1;
uniform vec4 u_transform = vec4(vec2(0), vec2(1));
uniform float u_aspectratio = 16.0/9.0;

void main()
{
	vec2 scale = u_transform.zw * vec2(u_aspectratio, 1);
	vec2 scaledTexCoords = v_texcoords * scale;
	vec2 centeredTexCoords = abs((scaledTexCoords - scale / 2) * 2);
	vec2 borderTexCoords = vec2(0);
	vec4 drawnColor = u_color;
	bool IsInEdge = false;
	if(centeredTexCoords.x > scale.x - u_borderScale)
	{
		IsInEdge = true;
		if(u_borderType == 2) drawnColor = u_color * 0.75;
	}
	if(centeredTexCoords.y > scale.y - u_borderScale)
	{
		if(u_borderType == 2) drawnColor = u_color * 0.75;
		if(IsInEdge && u_borderType == 1)
		{
			if(length((scale - u_borderScale) - centeredTexCoords) > u_borderScale) discard;
		}
	}
	if(u_offset.y > v_position.y)
	{
		discard;
	}
	if(u_offset.z < v_position.y)
	{
		discard;
	}
	if(u_usetexture == 1)
	{
		f_color = vec4(drawnColor.xyz, u_opacity) * texture(u_texture, v_texcoords);
	}
	else
	{
		f_color = vec4(drawnColor.xyz, u_opacity);
	}
}