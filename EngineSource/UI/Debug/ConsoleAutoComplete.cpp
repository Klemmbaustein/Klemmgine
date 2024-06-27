#if !SERVER
#include "ConsoleAutoComplete.h"
#include <Engine/Utility/StringUtility.h>

using namespace Debug;

ConsoleAutoComplete::ConsoleAutoComplete(TextRenderer* Font, float TextSize)
{
	this->Font = Font;
	this->TextSize = TextSize;
}

std::string Debug::ConsoleAutoComplete::CompleteSelection(std::string Command)
{
	auto Commands = GetRecommendations(Command);

	if (Commands.empty())
	{
		return Command;
	}

	if (Commands[SelectionIndex].IsComplete)
	{
		return Command;
	}
	else
	{
		return Commands[SelectionIndex].Name;
	}
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
				.IsComplete = true,
				.NameHighlight = SIZE_MAX,
				.Attributes = GetCommandArguments(i.second)
				});
		}
		else if (i.second.Name.substr(0, First.size()) == First)
		{
			Found.push_back(Recommendation{
				.Type = "<command> ",
				.Name = i.second.Name,
				.IsComplete = false,
				.NameHighlight = First.size(),
				.Attributes = GetCommandArguments(i.second)
				});

		}
	}

	for (const auto& i : Console::ConsoleSystem->ConVars)
	{
		bool IsEqual = i.second.Name == First;

		if (IsEqual || i.second.Name.substr(0, First.size()) == First)
		{
			Found.push_back(Recommendation{
				.Type = IsEqual ? "* <variable>" : "<variable>",
				.Name = i.second.Name,
				.IsComplete = IsEqual,
				.NameHighlight = First.size(),
				.Attributes =
				{
					"=" ,
					StrUtil::Format("(%s) %s",
						NativeType::TypeStrings[i.second.NativeType].c_str(),
						Console::GetVariableValueString(i.second).c_str())
				}
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
	size_t CommandIndex = 0;


	if (SelectionIndex > Found.size())
	{
		SelectionIndex = 0;
	}
	else if (SelectionIndex == Found.size())
	{
		SelectionIndex = Found.size() - 1;
	}

	for (auto& i : Found)
	{
		bool Selected = CommandIndex++ == SelectionIndex;

		std::vector<TextSegment> Segments = {
			TextSegment(i.Type + "  ", Selected ? 0.75f : 0.55f),
			TextSegment(i.Name.substr(0, i.NameHighlight), Selected ? Vector3(0.1f, 1.0f, 0.2f) : Vector3(1, 0.2f, 0.1f)),
		};

		if (i.NameHighlight < i.Name.size())
		{
			Segments.push_back(TextSegment(i.Name.substr(i.NameHighlight), Selected ? 1.0f : 0.75f));
		}

		for (const auto& attr : i.Attributes)
		{
			Segments.push_back(TextSegment(" " + attr, Selected ? 0.75f : 0.55f));
		}

		Target->AddChild((new UIText(TextSize, Segments, Font))->SetPadding(0, 0, 0.01f, 0.0f));
	}
}
#endif