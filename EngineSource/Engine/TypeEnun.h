#pragma once
#include <string>
#include <vector>

/**
* @file
*/

/**
* @brief
* Namespace containing information about types.
*/
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
	};

	/**
	* @brief
	* enum containing possible types.
	*/
	enum TypeEnum
	{
		/// No type.
		Null = -1,
		/// Vector3.
		Vector3 = 0,
		/// float.
		Float = 1,
		/// int32.
		Int = 2,
		/// string. Usually std::string.
		String = 3,
		/// Vector3. A UIVectorField will display 'R', 'G' 'B' instead of 'X', 'Y', 'Z' when editing a Vector3Color.
		Vector3Color = 4,
		/// Materials only. Texture information.
		GL_Texture = 5,
		/// uint8_t.
		Byte = 6,
		/// Boolean type. Either true or false. Editor will display this as a checkbox.
		Bool = 7,
		/// Vector3. A UIVectorField will display 'P', 'Y' 'R' (Pitch, Yaw, Roll) instead of 'X', 'Y', 'Z' when editing a Vector3Color.
		Vector3Rotation = 8,
		/// List modifier. Bitwise and this with any other value in this enum to make it a list.
		List = 0b10000000
	};
}