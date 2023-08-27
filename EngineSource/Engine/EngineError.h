#pragma once
#include <string>
#include <Engine/Utility/FileUtility.h>

#define __FILENAME__ FileUtil::GetFileNameFromPath(__FILE__)

namespace Engine
{
	
	void AssertFailure(std::string Name, std::string Location);
	
	inline void Assert(bool Value, std::string Name, std::string Location)
	{
		if (!Value)
		{
			AssertFailure(Name, Location);
		}
	}
}

// Crashes everything if the condition evaluates to false.
#define ENGINE_ASSERT(Cond, Description) Engine::Assert(Cond, Description + std::string("\nCondition: ") + #Cond + std::string(""), std::string(__FILENAME__) + ", " + std::string(__FUNCTION__) + ", Line " + std::to_string(__LINE__))
