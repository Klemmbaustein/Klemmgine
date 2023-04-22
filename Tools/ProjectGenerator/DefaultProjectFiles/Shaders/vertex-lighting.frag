//! #include "shared.frag"

in vec3 vertexColor;

void main()
{
	// in the fragment shader we just return the lighting calculated in the vertex shader.
	RETURN_COLOR(vertexColor);
}