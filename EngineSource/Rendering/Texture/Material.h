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
		std::string Description;
		Param(std::string UniformName, Type::TypeEnum Type, std::string Value, std::string Description = "")
		{
			this->UniformName = UniformName;
			this->Type = Type;
			this->Value = Value;
			this->Description = Description;
		}
	};

	std::vector<Param> Uniforms;
	std::string VertexShader = "basic.vert", FragmentShader = "basic.frag";
	bool UseShadowCutout = false, IsTranslucent = false;
	std::string Name;

	static void SetPredefinedMaterialValue(std::string Value, char* ptr, std::string Name);
	static Material LoadMaterialFile(std::string Name);
	static void SaveMaterialFile(std::string Path, Material m);

	static void ReloadMaterial(std::string MaterialPath);
};