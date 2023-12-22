#pragma once
#include <vector>
#include <unordered_map>
#include <Math/Vector.h>

namespace Input
{
    enum GamepadButton
    {
        GAMEPAD_INVALID = -1,
        GAMEPAD_A,
        GAMEPAD_B,
        GAMEPAD_X,
        GAMEPAD_Y,
        GAMEPAD_BACK,
        GAMEPAD_GUIDE,
        GAMEPAD_START,
        GAMEPAD_LEFTSTICK,
        GAMEPAD_RIGHTSTICK,
        GAMEPAD_LEFTSHOULDER,
        GAMEPAD_RIGHTSHOULDER,
        GAMEPAD_DPAD_UP,
        GAMEPAD_DPAD_DOWN,
        GAMEPAD_DPAD_LEFT,
        GAMEPAD_DPAD_RIGHT,
        GAMEPAD_MISC1,    /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button */
        GAMEPAD_PADDLE1,  /* Xbox Elite paddle P1 (upper left, facing the back) */
        GAMEPAD_PADDLE2,  /* Xbox Elite paddle P3 (upper right, facing the back) */
        GAMEPAD_PADDLE3,  /* Xbox Elite paddle P2 (lower left, facing the back) */
        GAMEPAD_PADDLE4,  /* Xbox Elite paddle P4 (lower right, facing the back) */
        GAMEPAD_TOUCHPAD, /* PS4/PS5 touchpad button */
        GAMEPAD_MAX
    };


	struct Gamepad
	{
        Gamepad();
		int32_t ID = 0;
		Vector2 LeftStickPosition = 0;
		Vector2 RightStickPosition = 0;
        Vector2 DPadLocation = 0;

        char* DeviceName = nullptr;

        bool IsButtonDown(GamepadButton b) const;

		float LeftBumper = 0, RightBumper = 0;
        bool* Buttons = nullptr;
    };

	extern std::unordered_map<int32_t, Gamepad> Gamepads;

	void AddGamepad(int32_t ID);

	void GamepadUpdate();
	void HandleGamepadEvent(void* EventPtr);
}