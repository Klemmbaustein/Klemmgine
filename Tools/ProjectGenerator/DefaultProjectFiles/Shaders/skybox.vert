//! #include "shared.vert"

uniform vec3 u_cameraposition;

void main()
{
	ReturnPosition(TranslatePosition(a_position + u_cameraposition / 50));
}