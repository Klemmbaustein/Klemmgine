#version 330

out vec2 v_texcoords;
out vec2 v_uitexcoords;
uniform bool FullScreen;

void main()
{
	float x = -1.f + float((gl_VertexID & 1) << 2);
	float y = -1.f + float((gl_VertexID & 2) << 1);
	
	v_uitexcoords.x = (x + 1.0) * 0.5f;
	v_uitexcoords.y = (y + 1.0) * 0.5f;
	gl_Position = vec4(x, y, -1.f, 1);
	if(!FullScreen)
	{
		x = x * 1.4;
	
		y = (((y + 1) * 1.5) - 1);
		
		y -= 0.6f;
	}
	v_texcoords = vec2((x + 1.0) * 0.5f, (y + 1.0) * 0.5f);
}