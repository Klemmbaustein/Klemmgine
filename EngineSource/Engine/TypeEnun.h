#pragma once
#include <string>
#include <vector>

namespace Type
{
	inline const std::vector<std::string> Types =
	{
		"vector3",
		"float",
		"int",
		"string",
		"color",
		"texture",
		"byte",
		"bool",
		"rotation",
		"list"
	};

	enum TypeEnum
	{
		Null = -1,
		Vector3 = 0,
		Float = 1,
		Int = 2,
		String = 3,
		Vector3Color = 4,
		GL_Texture = 5,
		Byte = 6,
		Bool = 7,
		Vector3Rotation = 8,
		List = 9
	};
}