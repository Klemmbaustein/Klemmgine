#pragma once
#include <string>
#include <Engine/TypeEnun.h>

struct LoadError : std::exception
{
	LoadError(std::string In)
	{
		Error = ("Loading Error: " + In);
	}
	const char* what() const noexcept override
	{
		return Error.c_str();
	}
	std::string Error;
};

Type::TypeEnum MaterialUniformTypeStringToInt(std::string Type);