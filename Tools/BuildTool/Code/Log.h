#pragma once
#include <string>

namespace Log
{
	enum class MessageType
	{
		Info = 0,
		Warning = 1,
		Error = 2
	};

	void SetIsVerbose(bool NewIsVerbose);
	bool GetIsVerbose();

	void SetConsoleColor(MessageType TypeColor);

	void Print(std::string Message, MessageType NativeType = MessageType::Info, std::string LogClass = "");
	void PrintVerbose(std::string Message);
}