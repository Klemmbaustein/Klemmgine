#include "Log.h"
#include <map>
#include <Engine/OS.h>
#include <iostream>

namespace Log
{
	std::map<OS::ConsoleColor, Vector3> ConsoleColors =
	{
		std::pair(OS::ConsoleColor::White, Log::LogColor::White),
		std::pair(OS::ConsoleColor::Gray, Log::LogColor::Gray),
		std::pair(OS::ConsoleColor::Red, Log::LogColor::Red),
		std::pair(OS::ConsoleColor::Green, Log::LogColor::Green),
		std::pair(OS::ConsoleColor::Blue, Log::LogColor::Blue),
		std::pair(OS::ConsoleColor::Yellow, Log::LogColor::Yellow),
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
		float NearestValue = 15;
		for (const auto& c : ConsoleColors)
		{
			float Difference = (c.second - Color).Length();
			if (NearestValue > Difference)
			{
				NearestColor = c.first;
				NearestValue = Difference;
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
