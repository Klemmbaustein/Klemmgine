#pragma once
#include <string>
#include <Rendering/Texture/Material.h>

namespace Preprocessor
{
	struct ProcessedShader
	{
		std::string Code;
		std::vector<Material::Param> ShaderParams;
	};

	// Preprocesses GLSL. For includes, a path from where includes should be searched for must be included.
	ProcessedShader ParseGLSL(const std::string& Code, std::string Path);
}