#version 330

layout(location = 0) out vec4 f_color;
in vec2 v_texcoords;
uniform sampler2D u_ui;
vec4 sampleUI()
{
	vec4 UIsample = vec4(0);
	vec2 texSize = 1.f / textureSize(u_ui, 0);
	UIsample += texture(u_ui, v_texcoords);
	UIsample += texture(u_ui, v_texcoords + vec2(0, texSize.y));
	UIsample += texture(u_ui, v_texcoords + vec2(texSize.x, 0));
	UIsample += texture(u_ui, v_texcoords + texSize);
	return UIsample / 4.f;
}
void main()
{
	f_color = sampleUI();
	//f_color = vec4(1);
}