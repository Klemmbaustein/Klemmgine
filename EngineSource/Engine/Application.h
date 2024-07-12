#pragma once

#if EDITOR
class EditorUI;
#endif

#include <set>
#include <Math/Vector.h>
#include <UI/UICanvas.h>

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

	extern bool ShowStartupInfo;

	int Initialize(int argc, char** argv);

	/**
	* @brief
	* DeInitializes the engine, quits the application.
	* 
	* If in the editor, this will call EditorUI::OnQuit().
	*/
	void Quit();


#if EDITOR
	/**
	* @brief
	Editor only. A pointer to the current EditorUI instance.
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