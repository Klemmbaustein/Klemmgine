#version 330

out vec2 v_texcoords;
out vec2 v_uitexcoords;
uniform vec2 Position = vec2(0);

void main()
{
	float x = -1.f + float((gl_VertexID & 1) << 2);
	float y = -1.f + float((gl_VertexID & 2) << 1);
	
	v_uitexcoords.x = (x + 1.0) * 0.5f;
	v_uitexcoords.y = (y + 1.0) * 0.5f;
	gl_Position = vec4(x, y, -1.f, 1);
	v_texcoords = vec2(x, y);
	v_texcoords -= Position;
	v_texcoords.x = (v_texcoords.x + 1.0) * 0.5f;
	v_texcoords.y = (v_texcoords.y + 1.0) * 0.5f;

}