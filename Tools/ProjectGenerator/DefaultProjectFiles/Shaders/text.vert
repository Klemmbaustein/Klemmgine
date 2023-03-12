#version 330 
layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 texcoords;// <vec2 pos, vec2 tex>
layout (location = 2) in vec3 color;
out vec2 TexCoords;
out vec2 v_position;
out vec3 v_color;
uniform vec3 u_offset; // X = Y offset; Y = MaxDistance; Z MinDistance
uniform vec3 transform;
uniform float u_aspectratio;
void main()
{
	vec2 pos = vertex; pos *= transform.z;
	pos += transform.xy;
	gl_Position = (vec4(pos / 450 / vec2(u_aspectratio, -1), 0.0, 1.0)) + vec4(0, -u_offset.x, 0, 0);
	v_position = gl_Position.xy;
	TexCoords = texcoords;
	v_color = color;
}