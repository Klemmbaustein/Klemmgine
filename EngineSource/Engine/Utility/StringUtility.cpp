#include "StringUtility.h"
#include <cstdarg>
#include <iostream>

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
	int Size = (int)Format.size() + 2, NewSize = Size;
	int Returned = 0;
	char* Buffer = nullptr;
	va_list va;
	va_start(va, Format);
	do
	{
		Size = NewSize;
		if (Buffer)
		{
			delete[] Buffer;
		}
		Buffer = new char[Size]();
		Returned = vsnprintf(Buffer, Format.size() + 250, Format.c_str(), va);
		NewSize = Returned;

	} while (Returned > Size);

	va_end(va);

	std::string StrBuffer = Buffer;
	delete[] Buffer;
	return StrBuffer;
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

std::vector<std::string> StrUtil::SeparateString(std::string Value, char Sep)
{
	std::vector<std::string> Values;

	std::string New;

	for (char c : Value)
	{
		if (c != Sep)
		{
			New.push_back(c);
		}
		else
		{
			Values.push_back(New);
			New.clear();
		}
	}

	if (!New.empty())
	{
		Values.push_back(New);
	}

	return Values;
}
