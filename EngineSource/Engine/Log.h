#pragma once
#include <string>
#include <Math/Vector.h>
namespace Log
{
	void Print(std::string Text, Vector3 Color = Vector3(1, 1, 1));
	void PrintMultiLine(std::string Text, Vector3 Color = Vector3(1, 1, 1), std::string Prefix = "");
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
	extern std::vector<Message> Messages;
	namespace LogColor
	{
		const inline Vector3 White = Vector3(1);
		const inline Vector3 Gray = Vector3(0.5f);
		const inline Vector3 Red = Vector3(1, 0.2f, 0);
		const inline Vector3 Green = Vector3(0.2f, 1, 0);
		const inline Vector3 Blue = Vector3(0.5, 0.6f, 1);
		const inline Vector3 Yellow = Vector3(1, 1, 0.2f);
	}
}