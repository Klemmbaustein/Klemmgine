#include "Input.h"

namespace Input
{
	Vector2 MouseLocation = Vector2(-2);
	bool CursorVisible = false;
	bool Keys[351];
}

bool Input::IsKeyDown(int Key)
{
	if (!(Key < 128))
	{
		Key -= 1073741755;
	}
	bool Test = Input::Keys[Key];
	return Test;
}

namespace Input
{
	bool IsLMBDown = false;
	bool IsRMBDown = false;
	Vector2 Input::MouseMovement;
}