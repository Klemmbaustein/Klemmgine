#include "Stats.h"
#include <vector>
#include "EngineProperties.h"

const bool IsInEditor = IS_IN_EDITOR;
const bool EngineDebug = ENGINE_DEBUG;


namespace Stats
{
	std::string VersionString = VERSION_STRING + std::string(IS_IN_EDITOR ? "-Editor" : "-Build");
	float DeltaTime;
	float FPS;
	float TimeMultiplier = 1;
	unsigned int DrawCalls = 0;
	float Time = 0;
	std::string EngineStatus;
	float LogicTime = 0, RenderTime = 0, SyncTime = 0;
	size_t FrameCount = 0;
}

namespace Editor
{
	bool IsInSubscene = false;
}
