#pragma once
#include <string>
#include <vector>

namespace StrUtil
{
	void ReplaceChar(std::string& Target, char A, std::string b);

	/**
	* @brief
	* Formats a string. Works like printf().
	*/
	std::string Format(std::string Format, ...);

	std::string VectorToString(std::vector<char> In);

	std::vector<std::string> SeperateString(std::string Value, char Sep);
}