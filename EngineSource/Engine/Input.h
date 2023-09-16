#pragma once
#include <Math/Vector.h>
#include <SDL.h>

namespace Input
{
	bool IsKeyDown(int Key);

	extern Vector2 MouseMovement;
	extern bool IsLMBDown;
	extern bool IsRMBDown;
	extern bool CursorVisible;
	extern Vector2 MouseLocation;
	extern bool BlockInput;
}

namespace TextInput
{
	extern bool PollForText;
	extern std::string Text;
	extern int TextIndex;
}