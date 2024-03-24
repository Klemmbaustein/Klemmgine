#include "Input.h"
#include "Log.h"

namespace Input
{
	Vector2 MouseLocation = Vector2(-2);
	bool CursorVisible = false;
	bool Keys[351];
	bool BlockInputConsole = false;
	bool BlockInput = false;
	bool IsLMBClicked = false;
	bool IsRMBClicked = false;
}

bool Input::IsKeyDown(Key InputKey)
{
	int KeyID = (int)InputKey;
	
	if (!(KeyID < 128))
	{
		KeyID -= 1073741755;
	}
	bool Test = Input::Keys[KeyID];
	if (BlockInput || BlockInputConsole)
	{
		Test = false;
	}

	return Test;
}

namespace Input
{
	bool IsLMBDown = false;
	bool IsRMBDown = false;
	Vector2 MouseMovement;
}

namespace TextInput
{
	bool BlockInput = false;
	bool PollForText = false;
	std::string Text;
	int TextIndex = 0;
	int TextSelectionStart = 0;
	std::string GetSelectedTextString()
	{
		int Start = std::min(TextIndex, TextSelectionStart), End = std::max(TextIndex, TextSelectionStart);
		return Text.substr(Start, End - Start);
	}
	void SetTextIndex(int NewIndex, bool ClearSelection)
	{
		TextIndex = NewIndex;
		if (ClearSelection)
		{
			TextSelectionStart = TextIndex;
		}
	}
}