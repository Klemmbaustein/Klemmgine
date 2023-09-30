#version 330
uniform sampler2D u_texture;
uniform sampler2D u_alpha;
uniform sampler2D u_hr;
uniform sampler2D u_hrAlpha;
in vec2 v_texcoords;

out vec4 f_color;

vec4 sampleHr(out float depth)
{
	vec4 UIsample = vec4(0);
	vec2 texSize = 1.f / textureSize(u_hr, 0);
	int divs = 0;
	float averageOpacity = 0;
	for (int x = 0; x < 2; ++x)
	{
		for (int y = 0; y < 2; ++y)
		{
			vec2 texCoords = v_texcoords + vec2(x, y) * texSize;
			vec4 newtex = texture(u_hr, texCoords);
			vec2 newAlpha = texture(u_hrAlpha, texCoords).rg;
			UIsample.w += newAlpha.r;
			if (newAlpha.r > 0.0)
			{
				depth = max(depth, newAlpha.g / newAlpha.r);
				averageOpacity += newAlpha.r;
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
	vec4 alpha = texture(u_alpha, v_texcoords);
	vec4 combined = vec4(color.xyz, alpha.r);

	float hrDepth = 0;
	vec4 hrColor = sampleHr(hrDepth);


	if (hrDepth > alpha.g)
	{
		if (combined.w > 0)
		{
			f_color.xyz = mix(combined.xyz, hrColor.xyz, clamp(hrColor.w / combined.w, 0, 1));
			f_color.w = clamp(combined.w + hrColor.w, 0, 1);
		}
		else
		{
			f_color = hrColor;
		}
	}
	else
	{		
		f_color = combined;
	}
}