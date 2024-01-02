#include "StringUtility.h"
#include <cstdarg>

void StrUtil::ReplaceChar(std::string& Target, char A, std::string b)
{
	std::string NewStr = Target;
	for (size_t i = 0; i < Target.size(); i++)
	{
		if (Target[i] == A)
		{
			NewStr.erase(i, 1);
			NewStr.insert(i, b);
			i--;
			i += b.size() + 1;
		}
	}
	Target = NewStr;
}

std::string StrUtil::Format(std::string Format, ...)
{
	char* buf = new char[Format.size() + 250ull]();
	va_list va;
	va_start(va, Format);
#if _WIN32
	vsprintf_s(buf, Format.size() + 250, Format.c_str(), va);
#else
	vsprintf(buf, Format.c_str(), va);
#endif
	va_end(va);
	return buf;
}


std::string StrUtil::VectorToString(std::vector<char> In)
{
	std::string Out;
	for (int i = 0; i < In.size(); i++)
	{
		Out.push_back(In.at(i));
	}
	return Out;
}