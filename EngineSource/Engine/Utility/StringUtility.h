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

	/**
	 * @brief
	 * Splits a string into different substring separated by a char.
	 * 
	 * Value = `"abc:def"`
	 * Sep = `':'`
	 * 
	 * returns: `{"abc", "def"}`
	 */
	std::vector<std::string> SeparateString(std::string Value, char Sep);

	std::string GetPrettyName(std::string Name);

	/**
	 * @brief
	 * Ensures the string is never longer than the given size.
	 * 
	 * Adds a ... to the string if it's too long.
	 */
	std::string ShortenIfTooLong(std::string str, size_t MaxSize);
}