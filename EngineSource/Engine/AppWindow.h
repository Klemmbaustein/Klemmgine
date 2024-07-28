#if !SERVER
#pragma once
#include <string>
#include <Math/Vector.h>

struct SDL_Window;

namespace Window
{
	void InitWindow(std::string WindowTitle);
	void DestroyWindow();

	void SetWindowTitle(std::string NewTitle);

	/**
	* @brief
	* Minimizes the application window.
	*/
	void Minimize();

	extern SDL_Window* SDLWindow;

	/**
	* @brief
	* A function that returns true if the application window has mouse focus.
	*
	* @return
	* true if the window has mouse focus, false if not.
	*/
	bool WindowHasFocus();

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
	* @return
	*
	* true if full screen is enabled, false if not.
	*/
	bool GetFullScreen();

	/**
	* @brief
	* Returns the size of the application window in pixels.
	*/
	Vector2 GetWindowSize();

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

	void SetTitleBarDark(bool NewIsDark);
}
#endif