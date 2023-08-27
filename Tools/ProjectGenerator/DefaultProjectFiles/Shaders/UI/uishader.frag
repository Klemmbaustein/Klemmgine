#version 330

in vec2 v_texcoords;
in vec2 v_position;
layout(location = 0) out vec4 f_color;
layout (location = 1) out vec4 f_alpha;  

uniform vec4 u_color;
uniform int u_usetexture;
uniform sampler2D u_texture;
uniform vec3 u_offset; // Scroll bar: X = scrolled distance; Y = MaxDistance; Z MinDistance
uniform float u_opacity;
uniform int u_borderType = 0;
uniform float u_borderScale = 0.05;
uniform vec4 u_transform;
uniform float u_aspectratio = 16.0/9.0;

void main()
{
	vec2 scale = u_transform.zw * vec2(u_aspectratio, 1);
	vec2 scaledTexCoords = v_texcoords * scale;
	vec2 centeredTexCoords = abs((scaledTexCoords - scale / 2) * 2);
	vec2 borderTexCoords = vec2(0);
	vec4 drawnColor = u_color;
	bool IsInEdge = false;
	float opacity = u_opacity;
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
			opacity *= clamp(1.0 / pow((length((scale - u_borderScale) - centeredTexCoords) / u_borderScale), 25), 0, 1);
		}
	}
	if(u_offset.y * 2 > v_position.y)
	{
		discard;
	}
	if(u_offset.z * 2 < v_position.y)
	{
		discard;
	}
	if(u_usetexture == 1)
	{
		vec4 tex = texture(u_texture, v_texcoords);
		opacity *= tex.w;
		f_color = vec4(drawnColor.xyz * tex.xyz, opacity);
	}
	else
	{
		f_color = vec4(drawnColor.xyz, opacity);
	}
	f_alpha = vec4(vec3(1), opacity);
}