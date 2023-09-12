#pragma once
#include <string>
#ifdef EDITOR
#define IS_IN_EDITOR true
#else
#define IS_IN_EDITOR false
#endif
#ifdef RELEASE
#define ENGINE_DEBUG false
#else
#define ENGINE_DEBUG true
#endif
namespace Project
{
	std::string GetStartupScene();
	void OnLaunch();
	extern const char* ProjectName;
}
#define VERSION_STRING "1.4.0-Alpha"
#define OPENGL_MIN_REQUIRED_VERSION "GL_VERSION_4_3"