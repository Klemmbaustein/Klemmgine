#version 330

in vec2 v_tex_coords;

uniform vec4 u_color;
uniform sampler2D u_texture;

out vec4 FragColor;

void main()
{
	FragColor = texture(u_texture, v_tex_coords) * u_color;
	if (FragColor.w == 0)
	{
		discard;
	}
}