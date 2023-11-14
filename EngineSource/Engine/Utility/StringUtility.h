#pragma once
#include <string>

namespace StrUtil
{
	void ReplaceChar(std::string& Target, char A, std::string b);
	std::string Format(std::string Format, ...);
}