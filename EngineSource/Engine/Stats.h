#pragma once
#include <string>

namespace Performance
{
	extern float DeltaTime;
	extern float FPS;
	extern float TimeMultiplier;
	extern unsigned int DrawCalls;
}

namespace Stats
{
	extern float Time;
}

namespace Editor
{
	extern bool IsInSubscene;
}

namespace Engine
{
	extern std::string CurrentProjectName;
	extern std::string VersionString;
}

namespace Debugging
{
	extern std::string EngineStatus;
}

const extern bool IsInEditor;
const extern bool EngineDebug;