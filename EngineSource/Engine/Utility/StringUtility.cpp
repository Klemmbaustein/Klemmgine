#include "StringUtility.h"
#include <cstdarg>

void StrReplace::ReplaceChar(std::string& Target, char A, std::string b)
{
	std::string NewStr = Target;
	for (size_t i = 0; i < Target.size(); i++)
	{
		if (Target[i] == A)
		{
			NewStr.erase(i, 1);
			NewStr.insert(i, b);
		}
	}
	Target = NewStr;
}

std::string StrUtil::Format(std::string Format, ...)
{
	char* buf = new char[Format.size() + 250ull]();
	va_list va;
	va_start(va, Format);
	vsprintf_s(buf, Format.size() + 250, Format.c_str(), va);
	va_end(va);
	return buf;
}
