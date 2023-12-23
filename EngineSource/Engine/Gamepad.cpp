#include "Gamepad.h"
#include <Engine/Log.h>
#include <Engine/Utility/StringUtility.h>

#include <SDL.h>
#include <iostream>

std::unordered_map<int32_t, Input::Gamepad> Input::Gamepads;
Vector2 GetHatState(Uint8 hat);

void Input::AddGamepad(int32_t ID)
{
	SDL_Joystick* New = SDL_JoystickOpen(ID);
	int32_t NewID = SDL_JoystickInstanceID(New);
	Log::Print(StrUtil::Format("Connected device '%s' with ID %i.", SDL_JoystickName(New), NewID));
	if (New == nullptr)
	{
		Log::Print(StrUtil::Format("Could not open gamepad %i - %s", NewID, SDL_GetError()));
	}
	else
	{
		Gamepad g;
		g.ID = NewID;
		g.DeviceName = SDL_strdup(SDL_JoystickName(New));
		g.Buttons = new bool[GAMEPAD_MAX]();
		Gamepads.insert(std::pair(NewID, g));
	}
}

void Input::GamepadUpdate()
{
}

void Input::HandleGamepadEvent(void* EventPtr)
{
	SDL_Event* e = (SDL_Event*)EventPtr;

	if (!Gamepads.contains(e->jdevice.which))
	{
		Log::Print(StrUtil::Format("Unknowm gamepad ID: %i", e->jdevice.which));
		return;
	}
	Gamepad& g = Gamepads[e->jdevice.which];

	switch (e->type)
	{
	case SDL_JOYBUTTONUP:
		g.Buttons[e->jbutton.button] = false;
		break;
	case SDL_JOYBUTTONDOWN:
		g.Buttons[e->jbutton.button] = true;
		break;
	case SDL_JOYHATMOTION:
		g.DPadLocation = GetHatState(e->jhat.hat);
		break;
	case SDL_JOYAXISMOTION:
		Uint8 StickIndex = e->jaxis.axis / 2;
		Uint8 Axis = e->jaxis.axis % 2;
		float AxisVal = (float)e->jaxis.value / (float)INT16_MAX;
		if (std::abs(AxisVal) < 0.1)
		{
			AxisVal = 0;
		}
		switch (StickIndex)
		{
		case 0:
			if (Axis == 0)
			{
				g.LeftStickPosition.X = AxisVal / 2.0f;
			}
			else
			{
				g.LeftStickPosition.Y = -AxisVal / 2.0f;
			}
			break;
		case 1:
			if (Axis == 0)
			{
				g.RightStickPosition.X = AxisVal / 2.0f;
			}
			else
			{
				g.RightStickPosition.Y = -AxisVal / 2.0f;
			}
			break;
		case 2:
			if (Axis == 0)
			{
				g.LeftBumper = std::max((AxisVal + 1.0f) / 2.0f, 0.0f);
			}
			else
			{
				g.RightBumper = std::max((AxisVal + 1.0f) / 2.0f, 0.0f);
			}
			break;

		default:
			break;
		}
		break;
	}
	//Log::Print(Gamepads[e->jdevice.which].LeftStickPosition.ToString());
	//Log::Print(StrUtil::Format("%u\t%s\t%u\t%s", timestamp, eventType.c_str(), index, otherData.c_str()));
}

Vector2 GetHatState(Uint8 hat) 
{
	switch (hat) 
	{
	case SDL_HAT_LEFTUP:
		return Vector2(-1, 1);
		break;
	case SDL_HAT_UP:
		return Vector2(0, 1);
		break;
	case SDL_HAT_RIGHTUP:
		return Vector2(1, 1);
		break;
	case SDL_HAT_LEFT:
		return Vector2(-1, 0);
		break;
	case SDL_HAT_CENTERED:
		return Vector2(0, 0);
		break;
	case SDL_HAT_RIGHT:
		return Vector2(1, 0);
		break;
	case SDL_HAT_LEFTDOWN:
		return Vector2(-1, -1);
		break;
	case SDL_HAT_DOWN:
		return Vector2(0, -1);
		break;
	case SDL_HAT_RIGHTDOWN:
		return Vector2(1, -1);
		break;
	default:
		return Vector2(0);
		break;
	}
}

Input::Gamepad::Gamepad()
{
}

bool Input::Gamepad::IsButtonDown(GamepadButton b) const
{
	return Buttons[b];
}
