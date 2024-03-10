//! #include "../shared.frag"
uniform vec3 u_color = vec3(1);

bool nearZero(float pos, float gridSize)
{
	float projectedPos = pos / gridSize;
	return projectedPos < 0.05 && projectedPos >= 0;
}

vec4 getColorOpacity(float gridSize)
{
	float colorMultiplier = 0.2;

	if (nearZero(v_position.x, gridSize) || nearZero(v_position.z, gridSize))
	{
		colorMultiplier = 1;
	}

	vec3 endColor = u_color * vec3(colorMultiplier);

	if (mod(v_position.x / gridSize, 1) > 0.05 && mod(v_position.z / gridSize, 1.0) > 0.05)
	{
		return vec4(endColor, 0);
	}

	return vec4(endColor, 1);
}

void main()
{
	float cameraDistance = distance(v_position, u_cameraposition);

	vec4 colorOpacity = mix(getColorOpacity(5), getColorOpacity(10), clamp(cameraDistance / 200, 0, 1));

	colorOpacity = mix(getColorOpacity(1), colorOpacity, clamp(cameraDistance / 80, 0, 1));

	if (colorOpacity.w < 0.4)
	{
		discard;
	}

	RETURN_COLOR_OPACITY(colorOpacity.xyz, max(min(colorOpacity.w / (cameraDistance / 100), 1) - 0.75, 0));
}