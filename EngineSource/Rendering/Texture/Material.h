#pragma once
#include <string>
#include <Engine/TypeEnun.h>
#include <vector>

struct Material
{
	struct Param
	{
		std::string UniformName;
		Type::TypeEnum Type;
		std::string Value;
		Param(std::string UniformName, Type::TypeEnum Type, std::string Value)
		{
			this->UniformName = UniformName;
			this->Type = Type;
			this->Value = Value;
		}
	};

	bool IsTemplate = false;

	std::vector<Param> Uniforms;
	std::string VertexShader = "basic.vert", FragmentShader = "basic.frag", Template;
	bool UseShadowCutout = false, IsTranslucent = false;

	static void SetPredefinedMaterialValue(std::string Value, char* ptr, std::string Name);
	static Material LoadMaterialFile(std::string Name, bool IsTemplate);
	static void SaveMaterialFile(std::string Path, Material m, bool IsTemplate);
};