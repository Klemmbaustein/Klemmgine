#include "Log.h"
#include <map>
#include <Engine/OS.h>
#include <iostream>

namespace Log
{
	std::map<OS::ConsoleColor, Vector3> ConsoleColors =
	{
		std::pair(OS::ConsoleColor::White, Vector3(1)),
		std::pair(OS::ConsoleColor::Gray, Vector3(0.5)),
		std::pair(OS::ConsoleColor::Red, Vector3(1, 0, 0)),
		std::pair(OS::ConsoleColor::Green, Vector3(0, 1, 0)),
		std::pair(OS::ConsoleColor::Blue, Vector3(0, 0, 1)),
		std::pair(OS::ConsoleColor::Yellow, Vector3(1, 1, 0)),
	};
	std::vector<Message> Messages;
	void Print(std::string Text, Vector3 Color)
	{
		if (Messages.size() > 0)
		{
			if (Messages.at(Messages.size() - 1).Text == Text && Messages.at(Messages.size() - 1).Color == Color)
			{
				Messages.at(Messages.size() - 1).Amount++;
			}
			else
			{
				Messages.push_back(Message(Text, Color));
			}
		}
		else
		{
			Messages.push_back(Message(Text, Color));
		}
		OS::SetConsoleColor(OS::ConsoleColor::Gray);
		OS::ConsoleColor NearestColor = OS::ConsoleColor::White;
		for (const auto& c : ConsoleColors)
		{
			if (Vector3::NearlyEqual(c.second, Color, 0.35f))
			{
				NearestColor = c.first;
			}
		}
		std::cout << "Log: ";
		OS::SetConsoleColor(NearestColor);
		std::cout << Text << std::endl;
		OS::SetConsoleColor(OS::ConsoleColor::Gray);
	}


	void PrintMultiLine(std::string Text, Vector3 Color, std::string Prefix)
	{
		std::string CurrentLine;
		for (auto& i : Text)
		{
			if (i == '\n')
			{
				Log::Print(Prefix + CurrentLine, Color);
				CurrentLine.clear();
			}
			else
			{
				CurrentLine.append({ i });
			}
		}
		if (CurrentLine.size())
		{
			Log::Print(Prefix + CurrentLine, Color);
		}
	}
}
