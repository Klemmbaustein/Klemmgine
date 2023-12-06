#version 430 
	
layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;
in vec2 v_tex_coord[];
out vec2 g_tex_coord;
layout (std140, binding = 0) uniform LightSpaceMatrices
{
	mat4 lightSpaceMatrices[8];
};
	
void main()
{          
	for (int i = 0; i < 3; ++i)
	{
		gl_Position = 
			lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
			g_tex_coord = v_tex_coord[i];
		gl_Layer = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();
}  