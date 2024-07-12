#pragma once
#include <string>

namespace Stats
{
	extern float DeltaTime;
	extern float FPS;
	extern float TimeMultiplier;
	extern unsigned int DrawCalls;
	extern float Time;
	extern std::string EngineStatus;

	extern size_t FrameCount;
	extern float LogicTime, RenderTime, SyncTime;
	extern std::string VersionString;
}

namespace Editor
{
	extern bool IsInSubscene;
}

const extern bool IsInEditor;
const extern bool EngineDebug;