#version 330

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texcoords;
uniform vec3 u_offset; //X = Y offset; Y = MaxDistance
uniform vec4 u_transform = vec4(vec2(0), vec2(1)); // xy = position zw = scale
out vec2 v_position;
out vec2 v_texcoords;

void main()
{
	gl_Position = vec4(a_position.xy * u_transform.zw + vec2(0, -u_offset.x) + u_transform.xy, 0, 1);
	v_texcoords = a_texcoords;
	v_position = gl_Position.xy;
}