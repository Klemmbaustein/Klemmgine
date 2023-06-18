#pragma once
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
extern char ProjectName[];
#define VERSION_STRING "1.2.2-Alpha"
#define OPENGL_MIN_REQUIRED_VERSION "GL_VERSION_4_3"