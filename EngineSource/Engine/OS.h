#pragma once
#include <string>

/*
* For the compilation to work on both Linux and Windows, 
* This header implements functions that are OS specific. This is
* (hopefully) the only part of the code that would require to be modified for the
* program to run on other OS's.
*/

namespace OS
{
	enum EConsoleColor
	{
		E_WHITE = 0,
		E_GRAY = 1,
		E_RED = 2,
		E_GREEN = 3,
		E_BLUE = 4,
		E_YELLOW = 5
	};
	void SetConsoleCanBeHidden(bool ConsoleCanBeHidden);
	void SetConsoleWindowVisible(bool Visible);
	std::string ShowOpenFileDialog();
	std::string GetOSString();
	void ClearConsoleWindow();
	void SetConsoleColor(EConsoleColor NewColor);
	void OpenFile(std::string Path);
}