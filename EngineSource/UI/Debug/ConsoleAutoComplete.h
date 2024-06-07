#if !SERVER
#pragma once
#include <UI/UICanvas.h>
#include <UI/UIText.h>
#include <Engine/Subsystem/Console.h>

namespace Debug
{
	class ConsoleAutoComplete
	{
		TextRenderer* Font = nullptr;
		float TextSize = 0;
	public:
		ConsoleAutoComplete(TextRenderer* Font, float TextSize);

		struct Recommendation
		{
			std::string Type;
			std::string Name;
			size_t NameHighlight = 0;
			std::vector<std::string> Attributes;
		};

		std::vector<Recommendation> GetRecommendations(std::string Command);
		std::vector<std::string> GetCommandArguments(const Console::Command& Command);

		void RenderToBox(UIBox* Target, const std::vector<Recommendation>& Found);
	};
}
#endif