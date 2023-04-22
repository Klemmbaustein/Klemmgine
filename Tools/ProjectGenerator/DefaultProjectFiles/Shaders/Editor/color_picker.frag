#version 330

in vec2 v_texcoords;
in vec2 v_position;
layout(location = 0) out vec4 f_color;

uniform vec3 selectedHue = vec3(1);
uniform int mode = 0;
uniform vec2 selectedPos = vec2(0);

bool NearlyEqual(float A, float B, float epsilon)
{
	return (abs(A - B) < epsilon);
}

vec3 rgb2hsv(vec3 c)
{
	vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
	vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

	float d = q.x - min(q.w, q.y);
	float e = 1.0e-10;
	return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
	if (mode == 0)
	{
		f_color = vec4(mix(vec3(0), mix(vec3(1), selectedHue, v_texcoords.x), v_texcoords.y), 1);
		float dst = length(selectedPos.xy - v_texcoords);
		if (dst < 0.03 && dst > 0.02)
		{
			f_color = vec4(1);
		}
	}
	else
	{
		vec3 color = hsv2rgb(vec3(v_texcoords.y, 1, 1));
		if (NearlyEqual(rgb2hsv(selectedHue).x, v_texcoords.y, 0.01))
		{
			f_color = vec4(1);
		}
		else
		{
			f_color = vec4(color, 1);
		}
	}
}