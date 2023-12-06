//! #include "shared.vert"

void main()
{
	ReturnPosition(TranslatePosition(a_position + u_cameraposition / 50));
}