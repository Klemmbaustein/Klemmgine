#version 330 
in vec2 TexCoords;
in vec2 v_position;
in vec3 v_color;
layout (location = 0) out vec4 color;
layout (location = 1) out vec4 f_alpha;  
uniform vec3 u_offset; //X = Y offset; Y = MaxDistance; Z MinDistance
uniform sampler2D u_texture;
uniform vec3 textColor;
uniform float u_opacity = 1.0f;
uniform float u_textSize = 0.0f;
uniform vec2 u_screenRes = vec2(1600, 900);
uniform vec3 transform;

#define NUM_SAMPLES 2

void main()
{   
	if(u_offset.y > v_position.y)
	{
		discard;
	}
	if(u_offset.z < v_position.y)
	{
		discard;
	}

	// TODO: Subpixel rendering (?)
	float sampled = 0;
	vec2 offset = (2.5 / transform.z) / u_screenRes;
	for (int x = -NUM_SAMPLES; x < NUM_SAMPLES; x++)
	{
		for (int y = -NUM_SAMPLES; y < NUM_SAMPLES; y++)
		{
			sampled += texture(u_texture, TexCoords + offset * vec2(x, y)).a;
		}
	}
	sampled /= NUM_SAMPLES * NUM_SAMPLES * 2 * 2;
	f_alpha.xyz = vec3(1, 1, 1);
	f_alpha.w = sampled * u_opacity;
	color = vec4(v_color, f_alpha.w);
}  