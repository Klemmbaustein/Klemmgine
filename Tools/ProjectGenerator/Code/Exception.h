#pragma once
#include <string>

class Exception
{
public:
	Exception(std::string Text, std::string Name = "Unknown Error");
	std::string What();
protected:
	std::string Name;
	std::string Text;
};