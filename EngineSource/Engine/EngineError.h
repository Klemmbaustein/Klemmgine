#pragma once
#include <string>
#include <Engine/Utility/FileUtility.h>

#define __FILENAME__ FileUtil::GetFileNameFromPath(__FILE__)

namespace Error
{
	void Init();
	
	void AssertFailure(std::string Name, std::string Location);
	
	inline void Assert(bool Value, std::string Name, std::string Location)
	{
		if (!Value)
		{
			AssertFailure(Name, Location);
		}
	}

	void PrintStackTrace();
}

// Crashes everything if the condition evaluates to false.
#define ENGINE_ASSERT(Cond, Description) Error::Assert(Cond, Description + std::string("\nCondition: ") + #Cond + std::string(""), std::string(__FILENAME__) + ", " + std::string(__FUNCTION__) + ", Line " + std::to_string(__LINE__))
