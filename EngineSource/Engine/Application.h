#pragma once

#if EDITOR
class EditorUI;
#endif

#include <set>
#include <Math/Vector.h>

struct SDL_Window;
struct ButtonEvent;

/**
* @file
* @brief
* File containing functions to manage the application itself.
*/

/**
* @brief
* Namespace containing functions to manage the application itself.
*/
namespace Application
{
	extern std::string StartupSceneOverride;

	/**
	* @brief
	* A function that returns true if the application window has mouse focus.
	* @returns
	* true if the window has mouse focus, false if not.
	*/
	bool WindowHasFocus();

	extern bool ShowStartupInfo;

	int Initialize(int argc, char** argv);

	extern float LogicTime, RenderTime, SyncTime;
	extern SDL_Window* Window;

	/**
	* @brief
	* Deinitializes the engine, quits the application.
	* 
	* If in the editor, this will call EditorUI::OnQuit().
	*/
	void Quit();

	/**
	* @brief
	* Minimizes the application window.
	*/
	void Minimize();

	/**
	* @brief
	* Enables/disables full screen mode.
	* 
	* For full screen, SDL_WINDOW_FULLSCREEN_DESKTOP is used.
	* 
	* @param NewFullScreen
	* If true, the application window will be set to full screen. If false, the window will be in windowed mode.
	*/
	void SetFullScreen(bool NewFullScreen);

	/**
	* @brief
	* A function that returns true if full screen is enabled.
	* @returns
	* true if full screen is enabled, false if not.
	*/
	bool GetFullScreen();

	/**
	* @brief
	* Sets the mouse cursors position to the given position.
	* 
	* Expects coordinates from -1, -1 (bottom left corner of the window) to 1, 1 (top right corner of the window)
	* 
	* @param NewPos
	* The new position of the cursor.
	*/
	void SetCursorPosition(Vector2 NewPos);

	/**
	* @brief
	* Returns the position of the mouse cursor.
	* 
	* The position will be in the format: -1, -1 (bottom left corner of the window) to 1, 1 (top right corner of the window)
	* 
	* @return
	* The current position of the cursor.
	*/
	Vector2 GetCursorPosition();

	/**
	* @brief
	* Returns the size of the application window in pixels.
	*/
	Vector2 GetWindowSize();
#if EDITOR
	/**
	* @brief Editor only. A pointer to the current EditorUI instance.
	*/
	extern EditorUI* EditorInstance;
#endif
	void SetEditorPath(std::string NewEditorPath);

	/**
	* @brief Gets the path to the editor files ({Path}/EditorContent/, {Path}/CSharp/)
	* 
	* @return
	* Path to the editor files.
	*/
	std::string GetEditorPath();

	extern std::set<ButtonEvent> ButtonEvents;

	void FreeOcclusionQuery(uint8_t Index);

	/**
	* @brief
	* A timer class that accurately measures time since it's creation.
	* 
	* This class uses SDL_GetPerformanceCounter() to accurately measure time.
	*/
	struct Timer
	{
		Timer();
		/**
		* @brief
		* Resets the timer.
		*/
		void Reset();
		/**
		* @brief
		* Returns the time in seconds since construction or the last call to Reset().
		*/
		float Get() const;
	private:
		uint64_t Time = 0;
	};
}