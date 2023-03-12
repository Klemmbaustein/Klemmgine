#pragma once
#include <string>

namespace Log
{
	enum EMessageType
	{
		E_INFO = 0,
		E_WARNING = 1,
		E_ERROR = 2
	};

	void SetIsVerbose(bool NewIsVerbose);
	bool GetIsVerbose();

	void Print(std::string Message, EMessageType Type = E_INFO);
	void PrintVerbose(std::string Message);
}