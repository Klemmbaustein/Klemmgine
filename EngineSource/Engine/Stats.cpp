#include "Stats.h"
#include <vector>
#include "EngineProperties.h"

namespace Engine
{
	std::string CurrentProjectName = ProjectName;
	std::string VersionString = VERSION_STRING + std::string(IS_IN_EDITOR ? "-Editor" : "-Build");
}

const bool IsInEditor = IS_IN_EDITOR;
const bool EngineDebug = ENGINE_DEBUG;


namespace Performance
{
	float DeltaTime;
	float FPS;
	float TimeMultiplier = 1;
	unsigned int DrawCalls = 0;
}


namespace Stats
{
	float Time = 0;
}


namespace Debugging
{
	std::string EngineStatus;
}

namespace Editor
{
	bool IsInSubscene = false;
}