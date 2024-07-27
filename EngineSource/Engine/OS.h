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
	enum class ConsoleColor
	{
		White = 0,
		Gray = 1,
		Red = 2,
		Green = 3,
		Blue = 4,
		Yellow = 5
	};

	size_t GetMemUsage();

	std::wstring Utf8ToWstring(std::string utf8);

	std::string GetExecutableName();

	void SetConsoleCanBeHidden(bool ConsoleCanBeHidden);
	void SetConsoleWindowVisible(bool Visible);
	std::string ShowOpenFileDialog();
	std::string GetOSString();
	void SetConsoleColor(ConsoleColor NewColor);
	void OpenFile(std::string Path);
}