#version 330
uniform sampler2D u_texture;
uniform sampler2D u_alpha;
uniform sampler2D u_hr;
uniform sampler2D u_hrAlpha;
in vec2 v_texcoords;

out vec4 f_color;

/*
* This shader merges a High res (hr) and normal UI framebuffer together.
*
* TODO: Un-break
*	This is a bad idea. It should instead render high res graphics 
*	into a small buffer per rendered object, then put it into the main
*	buffer immediately, not merging the 2 buffers at post processing.
*	This way completely breaks when transparency gets involved.
*	There's probably going to be performance issues using that method...
*
*	Until then: Do *not* render transparent UI over text or images.
*/

vec4 sampleHr(vec4 inColor, float inAlpha, float inDepth, out float depth)
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
			float pixelDepth = newAlpha.g / newAlpha.r;
			if (newAlpha.r > 0.0)
			{
				if (pixelDepth < inDepth)
				{
					// This breaks.
					UIsample.xyz += vec3(mix(newtex.xyz, inColor.xyz, inAlpha));
					averageOpacity += clamp(inColor.w + inAlpha, 0, 1);
					++divs;
				}
				else
				{
					depth = max(depth, pixelDepth);
					averageOpacity += newAlpha.r;
					UIsample.xyz += newtex.xyz;
					++divs;
				}
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

	// color.w is going to be inaccurate for actual transparency and doesn't
	// consider multiple transparent elements layered on eachother.
	vec4 hrColor = sampleHr(combined, color.w, alpha.g, hrDepth);

	if (alpha.g > hrDepth)
	{
		// This is a hack and will break.
		if (hrColor.w > 0)
		{
			f_color.xyz = mix(hrColor.xyz, hrColor.xyz, color.w);
		}
		else
		{
			f_color.xyz = combined.xyz;
		}
	}
	else
	{
		f_color.xyz = mix(combined.xyz, hrColor.xyz, clamp(hrColor.w, 0, 1));
	}
	f_color.w = clamp(combined.w + hrColor.w, 0, 1);
}