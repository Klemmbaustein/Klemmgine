#pragma once
#include "MaterialFunctions.h"

Type::TypeEnum MaterialUniformTypeStringToInt(std::string MType)
{
	if (MType == "Integer" || MType == "int" || MType == "num" || MType == "Int" || MType == "default" || MType == "bool" || MType == "i")
	{
		return Type::Int;
	}
	if (MType == "Float" || MType == "float" || MType == "double" || MType == "f")
	{
		return Type::Float;
	}
	if (MType == "Vector2" || MType == "vec2" || MType == "Vec2" || MType == "vector2")
	{
		return Type::Bool;
	}
	if (MType == "Vector3" || MType == "vec3" || MType == "Vec3" || MType == "vec" || MType == "vector3")
	{
		return Type::Bool;
	}
	if (MType == "Vector4" || MType == "vec4" || MType == "Vec4" || MType == "vector4")
	{
		return Type::Bool;
	}
	if (MType == "Texture" || MType == "tex" || MType == "Tex" || MType == "texture")
	{
		return Type::GL_Texture;
	}
	return Type::Bool;
}