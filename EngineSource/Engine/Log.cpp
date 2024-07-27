#include "Log.h"
#include <map>
#include <Engine/OS.h>
#include <iostream>
#include <mutex>

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

	std::mutex LogMutex;
	std::vector<Message> Messages;
	bool PrintColor = true;

	void EnableColoredOutput(bool NewEnabled)
	{
		PrintColor = NewEnabled;
	}

	void Print(std::string Text, Vector3 Color)
	{
		LogMutex.lock();
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
		if (PrintColor)
			OS::SetConsoleColor(OS::ConsoleColor::Gray);
		std::cout << "Log: ";
		if (PrintColor)
			OS::SetConsoleColor(NearestColor);
		std::cout << Text;
		if (PrintColor)
			OS::SetConsoleColor(OS::ConsoleColor::Gray);
		std::cout << std::endl;
		LogMutex.unlock();
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
	std::vector<Message> GetMessages()
	{
		LogMutex.lock();
		auto MessageCopy = Messages;
		LogMutex.unlock();
		return MessageCopy;
	}
}
