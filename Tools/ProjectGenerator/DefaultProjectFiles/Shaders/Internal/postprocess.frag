#version 330

layout(location = 0) out vec4 f_color;

in vec2 v_texcoords;
in vec2 v_uitexcoords;
float Vignette = length((vec2(-0.5) + v_texcoords) / 2);

uniform sampler2D u_texture;
uniform sampler2D u_outlines;
uniform sampler2D u_enginearrows;
uniform sampler2D u_bloomtexture;
uniform sampler2D u_ssaotexture;
uniform sampler2D u_depth;
uniform sampler2D u_ui;
uniform float u_gamma = 1;
uniform float u_chrabbsize = 0;
uniform float u_vignette = 1;
uniform bool u_fxaa = false;
uniform bool u_ssao = false;
uniform bool u_bloom = false;
uniform float u_time = 0;
uniform bool u_editor = false;
uniform bool u_hasWindowBorder = false;
uniform vec3 u_borderColor = vec3(1);

#define DEPTH_MAX 10000

float rand(vec2 co){
	return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}


float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0; // Back to NDC 
	return (2.0 * 0.1f * DEPTH_MAX) / (DEPTH_MAX + 0.1f - z * (DEPTH_MAX - 0.1f));
}
float blurssao()
{
	vec2 texelSize = 1.f / vec2(textureSize(u_ssaotexture, 0));
	float result = 0.0;
	for (int x = -2; x < 2; ++x) 
	{
		for (int y = -2; y < 2; ++y) 
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize + texelSize / 2;
			result += texture(u_ssaotexture, v_texcoords + offset).r;
		}
	}
	result = mix(result, 16, min(max(LinearizeDepth(texture(u_depth, v_texcoords).z), 0), 1) / 3);
	result += LinearizeDepth(texture(u_depth, v_texcoords).z) / 6.f;
	return pow(min((result + 1) / 16, 1), 1.1);
}

vec4 sampleUI()
{
	vec4 UIsample = vec4(0);
	vec2 texSize = 1.f / textureSize(u_ui, 0);
	int divs = 0;
	float averageOpacity = 0;
	for (int x = 0; x < 2; ++x)
	{
		for (int y = 0; y < 2; ++y)
		{
			vec4 newtex = texture(u_ui, v_uitexcoords + vec2(x, y) * texSize);
			UIsample.w += newtex.w;
			if (newtex.w > 0.0)
			{
				averageOpacity += newtex.w;
				UIsample.xyz += newtex.xyz;
				++divs;
			}
		}
	}
	if (divs != 0)
	{
		UIsample.xyz /= divs;
		averageOpacity /= divs;
		UIsample.w /= 4;
		UIsample.xyz /= averageOpacity;
	}
	UIsample.w = clamp(UIsample.w, 0, 1);
	return UIsample;
}

void main()
{
	vec4 color = texture(u_texture, v_texcoords);
	color *= 1.25;
	vec2 texelSize = 1.f / textureSize(u_texture, 0);
	if (u_ssao) color *= blurssao();
	vec3 outlinecolor = vec3(0.f);
	float outlinelevel = LinearizeDepth(texture(u_outlines, v_texcoords).x);
	vec4 uicolor = sampleUI();



	if (u_editor)
	{
		for (int x = -1; x <= 1; x += 2)
		{
			for (int y = -1; y <= 1; y += 2)
			{
			
				float difference = outlinelevel - LinearizeDepth(texture(u_outlines, v_texcoords + vec2(x, y) * texelSize * 4).x);
				if (difference > (outlinelevel / DEPTH_MAX * 1000))
				{
					outlinecolor += difference / 100;
				}
			}
		}
	}

	if (u_hasWindowBorder)
	{
		vec2 EdgeSize = vec2(1.0) / textureSize(u_texture, 0);
		if (v_uitexcoords.x <= EdgeSize.x)
		{
			uicolor = vec4(u_borderColor, 1);
		}
		if (v_uitexcoords.y <= EdgeSize.y)
		{
			uicolor = vec4(u_borderColor, 1);
		}
		if (1.0 - v_uitexcoords.x <= EdgeSize.x)
		{
			uicolor = vec4(u_borderColor, 1);
		}
		if (1.0 - v_uitexcoords.y <= EdgeSize.y)
		{
			uicolor = vec4(u_borderColor, 1);
		}
	}

	vec3 bloomcolor = vec3(0);
	if (u_bloom)
	{
		bloomcolor = texture(u_bloomtexture, v_texcoords).xyz;
	}

	vec4 enginearrows = texture(u_enginearrows, v_texcoords);
	float bloomstrength = clamp(length(bloomcolor) / 4.5, 0, 1);
	color = vec4(mix(color.xyz, bloomcolor, clamp(bloomstrength, 0, 1)), 1);
	f_color = pow(vec4(color.xyz + outlinecolor, color.w), vec4(u_gamma));

	f_color = mix(f_color, enginearrows, length(enginearrows.rgb));
	f_color *= (rand(v_texcoords) / 50) + 0.95; // To combat color banding
	f_color -= Vignette * u_vignette;
	f_color.xyz = mix(clamp(f_color.xyz, 0, 1), uicolor.xyz, clamp(uicolor.w, 0, 1));
	//f_color = uicolor;
	f_color.w = 1;
}