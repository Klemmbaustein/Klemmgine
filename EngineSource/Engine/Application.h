#pragma once

#if EDITOR
class EditorUI;
#endif

#include <set>
#include <Math/Vector.h>

struct SDL_Window;
struct ButtonEvent;

namespace Application
{

	bool WindowHasFocus();

	extern float LogicTime, RenderTime, SyncTime;
	extern SDL_Window* Window;
	void Quit();
	void SetFullScreen(bool NewFullScreen);
	bool GetFullScreen();
	void SetCursorPosition(Vector2 NewPos);
	Vector2 GetCursorPosition();
	Vector2 GetWindowSize();
#if EDITOR
	extern EditorUI* EditorUserInterface;
#endif

	extern std::set<ButtonEvent> ButtonEvents;
	struct Timer
	{
		Timer();
		void Reset();
		float TimeSinceCreation();
	private:
		uint64_t Time = 0;
	};
}