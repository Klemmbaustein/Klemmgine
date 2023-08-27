#pragma once
#include <string>

namespace Preprocessor
{
	// Preprocesses GLSL. For includes, a path from where includes should be searched for must be included.
	std::string ParseGLSL(std::string Code, std::string Path);
}