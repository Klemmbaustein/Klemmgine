#pragma once
#include <string>
#include <Math/Vector.h>

/**
* @file
* Functions related to Logging.
*/

/**
* @brief 
* Namespace containing Logging functions
* 
* C# equivalent: Engine.Log
*/
namespace Log
{
	/**
	* Contains color constants for Log::Print().
	*/
	namespace LogColor
	{
		const inline Vector3 White = Vector3(1);
		const inline Vector3 Gray = Vector3(0.75f);
		const inline Vector3 Red = Vector3(1, 0.2f, 0);
		const inline Vector3 Green = Vector3(0.2f, 1, 0);
		const inline Vector3 Blue = Vector3(0.5, 0.6f, 1);
		const inline Vector3 Yellow = Vector3(1, 1, 0.2f);
	}

	void EnableColoredOutput(bool NewEnabled);

	/**
	* Prints the given string with the given string to standard output and to the log display (if avaliable).
	* For colors, you should use the constants in Log::LogColor. These colors are mapped to terminal colors when output.
	* 
	* @param Text 
	* The string to print.
	* 
	* @param Color
	* The color the Text should have. Should be one of the constants in Log::LogColor, but can be anything.
	*/
	void Print(std::string Text, Vector3 Color = Vector3(1, 1, 1));

	/**
	* Like Log::Print(), but respects \\n. \\r\\n is not supported.
	* 
	* @param Text
	* The string to print.
	* 
	* @param Color
	* The color the Text should have. Should be one of the constants in Log::LogColor, but can be anything.
	*
	* @param Prefix
	* A prefix string that should be printed before each line.
	*/
	void PrintMultiLine(std::string Text, Vector3 Color = LogColor::White, std::string Prefix = "");

	struct Message
	{
		Vector3 Color;
		std::string Text;
		int Amount = 0;
		Message(std::string Text, Vector3 Color)
		{
			this->Color = Color;
			this->Text = Text;
		}
	};
	
	std::vector<Message> GetMessages();
}