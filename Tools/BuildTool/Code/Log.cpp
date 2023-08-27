#include "Log.h"
#include <iostream>

#if _WIN32
#include <Windows.h>
#include <wincon.h>

bool IsVerbose = false;
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

std::string MessageTypeStrings[3] =
{
	"[Info]",
	"[Warning]",
	"[Error]",
};
unsigned int MessageTypeMajorColors[3] =
{
	FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	FOREGROUND_RED | FOREGROUND_INTENSITY
};

unsigned int MessageTypeMinorColors[3] =
{
	BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY,
	BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY,
	BACKGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED
};

void Log::SetIsVerbose(bool NewIsVerbose)
{
	IsVerbose = NewIsVerbose;
}

bool Log::GetIsVerbose()
{
	return IsVerbose;
}

void Log::Print(std::string Message, MessageType Type, std::string LogClass)
{
	if (LogClass.empty())
	{
		LogClass = MessageTypeStrings[(int)Type];
	}
	SetConsoleTextAttribute(hConsole, MessageTypeMinorColors[(int)Type]);
	std::cout << LogClass;
	SetConsoleTextAttribute(hConsole, MessageTypeMajorColors[(int)Type]);
	std::cout << ": " << Message << std::endl;
	SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void Log::PrintVerbose(std::string Message)
{
	if (IsVerbose)
	{
		Log::Print(Message);
	}
}
#elif __linux__

void Log::Print(std::string Message, MessageType Type, std::string LogClass)
{
	if (LogClass.empty())
	{
		LogClass = MessageTypeStrings[(int)Type];
	}
	std::cout << LogClass;
	std::cout << ": " << Message << std::endl;
}
#endif