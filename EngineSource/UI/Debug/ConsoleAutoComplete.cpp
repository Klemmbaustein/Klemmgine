#if !SERVER
#include "ConsoleAutoComplete.h"
#include <Engine/Utility/StringUtility.h>

using namespace Debug;

ConsoleAutoComplete::ConsoleAutoComplete(TextRenderer* Font, float TextSize)
{
	this->Font = Font;
	this->TextSize = TextSize;
}

std::vector<ConsoleAutoComplete::Recommendation> ConsoleAutoComplete::GetRecommendations(std::string Command)
{
	std::vector<Recommendation> Found;
	
	std::vector CommandElements = Console::SeparateToStringArray(Command);

	if (CommandElements.empty())
	{
		return {};
	}

	const std::string& First = CommandElements[0];

	for (const auto& i : Console::ConsoleSystem->Commands)
	{
		if (i.second.Name == First)
		{
			Found.insert(Found.begin(), Recommendation{
				.Type = "* <command> ",
				.Name = i.second.Name,
				.NameHighlight = SIZE_MAX,
				.Attributes = GetCommandArguments(i.second)
				});
		}
		else if (i.second.Name.substr(0, First.size()) == First)
		{
			Found.insert(Found.begin(), Recommendation{
				.Type = "<command> ",
				.Name = i.second.Name,
				.NameHighlight = First.size(),
				.Attributes = GetCommandArguments(i.second)
				});

		}
	}

	for (const auto& i : Console::ConsoleSystem->ConVars)
	{
		if (i.second.Name == First)
		{
			Found.insert(Found.begin(), Recommendation{
				.Type = "* <variable>",
				.Name = i.second.Name,
				.NameHighlight = SIZE_MAX,
				.Attributes = { "=" , NativeType::TypeStrings[i.second.NativeType]}
				});
		}
		else if (i.second.Name.substr(0, First.size()) == First)
		{
			Found.insert(Found.begin(), Recommendation{
				.Type = "<variable>",
				.Name = i.second.Name,
				.NameHighlight = First.size(),
				.Attributes = { "=" , StrUtil::Format("(%s)", NativeType::TypeStrings[i.second.NativeType].c_str())}
				});

		}
	}

	return Found;
}

std::vector<std::string> ConsoleAutoComplete::GetCommandArguments(const Console::Command& Command)
{
	std::vector<std::string> Found;

	for (const auto& Argument : Command.Arguments)
	{
		std::string TypeString = NativeType::TypeStrings[Argument.NativeType];

		if (Argument.Optional)
		{
			TypeString = "optional " + TypeString;
		}

		std::string OutString = StrUtil::Format("(%s) %s", TypeString.c_str(), Argument.Name.c_str());

		if (&Argument != &Command.Arguments.at(Command.Arguments.size() - 1))
		{
			OutString.push_back(',');
		}

		Found.push_back(OutString);
	}

	return Found;
}

void Debug::ConsoleAutoComplete::RenderToBox(UIBox* Target, const std::vector<Recommendation>& Found)
{
	Target->DeleteChildren();
	for (auto& i : Found)
	{
		std::vector<TextSegment> Segments = {
			TextSegment(i.Type + "  ", 0.75f),
			TextSegment(i.Name.substr(0, i.NameHighlight), Vector3(1, 0.2f, 0.1f)),
		};

		if (i.NameHighlight < i.Name.size())
		{
			Segments.push_back(TextSegment(i.Name.substr(i.NameHighlight), 1));
		}

		for (const auto& attr : i.Attributes)
		{
			Segments.push_back(TextSegment(" " + attr, 0.75f));
		}

		Target->AddChild((new UIText(TextSize, Segments, Font))->SetPadding(0));
	}
}
#endif