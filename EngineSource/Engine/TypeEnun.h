#pragma once
#include <string>

namespace Type
{
	inline constexpr std::string Types[] =
	{
		"vector3",
		"float",
		"int",
		"string",
		"vector3_color",
		"texture",
		"byte",
		"bool"
	};

	enum TypeEnum
	{
		E_NULL = -1,
		E_VECTOR3 = 0,
		E_FLOAT = 1,
		E_INT = 2,
		E_STRING = 3,
		E_VECTOR3_COLOR = 4,
		E_GL_TEXTURE = 5,
		E_BYTE = 6,
		E_BOOL = 7
	};
}