#pragma once

#if EDITOR
class EditorUI;
#endif

#include <set>

struct SDL_Window;
struct ButtonEvent;

namespace Application
{
	extern SDL_Window* Window;
	void Quit();
	void SetFullScreen(bool NewFullScreen);
	bool GetFullScreen();

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