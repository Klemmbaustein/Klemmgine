#if EDITOR
#include "LogUI.h"
#include <UI/UIScrollBox.h>
#include <UI/UIText.h>
#include <Engine/Log.h>
#include <UI/EditorUI/EditorUI.h>
#include <UI/UITextField.h>
#include <Engine/Subsystem/Console.h>
#include <Engine/Input.h>
#include <UI/UIText.h>
#include <Engine/Application.h>

static float LogTextSize = 0.38f;
static std::vector<UIBox*> LogElements;

void LogUI::UpdateLogBoxSize()
{
	LogPrompt->SetColor(EditorUI::UIColors[1] * 0.5f);
	LogScrollBox->SetMinSize((Scale - Vector2(0.06f, 0.1f)));
	LogScrollBox->SetMaxSize((Scale - Vector2(0.06f, 0.1f)));
}

void LogUI::UpdateAutoComplete()
{
	LastCommand = LogPrompt->GetText();

	AutoComplete->RenderToBox(CommandHighlightScrollBox, AutoComplete->GetRecommendations(LastCommand));
	CommandsBackground->SetPosition(LogPrompt->GetPosition() + Vector2(0, LogPrompt->GetUsedSize().Y));
	CommandsBackground->SetMinSize(Vector2(LogPrompt->GetUsedSize().X, 0));
}

LogUI::LogUI(EditorPanel* Parent) : EditorPanel(Parent, "Console", "log")
{
	LogScrollBox = new UIScrollBox(UIBox::Orientation::Vertical, 0, true);
	LogPrompt = new UITextField(0, EditorUI::UIColors[1] * 0.5f, this, 0, EditorUI::MonoText);
	LogPrompt->HintText = "Console";
	PanelMainBackground->AddChild((new UIBackground(UIBox::Orientation::Vertical, 0, EditorUI::UIColors[1], 0))
		->SetPadding(0.02f)
		->AddChild(LogScrollBox
			->SetScrollSpeed(4)
			->SetPadding(0, 0, 0.01f, 0))
		->AddChild(LogPrompt
			->SetTextSize(0.45f)
			->SetTryFill(true)));

	AutoComplete = new Debug::ConsoleAutoComplete(EditorUI::MonoText, 0.425f);

	CommandsBackground = new UIBackground(UIBox::Orientation::Vertical, 0, 0, Vector2(1, 0));
	CommandHighlightScrollBox = new UIScrollBox(UIBox::Orientation::Vertical, 0, true);
	CommandHighlightScrollBox
		->SetTryFill(true);
	CommandsBackground
		->SetOpacity(0.95f)
		->AddChild(CommandHighlightScrollBox);

	UpdateLogBoxSize();
}

void LogUI::OnResized()
{
	UpdateLogBoxSize();
	if (LogTexts.size())
	{
		for (UIText* i : LogTexts)
		{
			i->SetWrapEnabled(true, 1.75f * LogScrollBox->GetUsedSize().X, UIBox::SizeMode::ScreenRelative);
		}
		ResetScroll();
	}
}

void LogUI::PrintUIElement(UIBox* Element)
{
	auto AllLogs = EditorUI::GetAllInstancesOf<LogUI>();

	for (LogUI* i : AllLogs)
	{
		i->Tick();
		i->LogScrollBox->AddChild(Element);
		return;
	}
	LogElements.push_back(Element);
}

void LogUI::ResetScroll()
{
	if (LogTexts.size())
	{
		// Update positions of everything first
		LogScrollBox->UpdateSelfAndChildren();
		LogScrollBox->Tick();

		float TextDifference = LogScrollBox->GetPosition().Y - LogTexts.at(LogTexts.size() - 1)->GetPosition().Y;
		LogScrollBox->GetScrollObject()->Percentage = LogScrollBox->GetScrollObject()->MaxScroll;
		LogScrollBox->RedrawElement();
	}
}

void LogUI::OnButtonClicked(int Index)
{
	HandlePanelButtons(Index);
	if (Index == 0)
	{
		if (Input::IsKeyDown(Input::Key::RETURN))
		{
			Log::Print("> " + LogPrompt->GetText(), Vector3(0.3f, 0.6f, 1));
			Console::ExecuteConsoleCommand(LogPrompt->GetText());
		}
		LogPrompt->SetText("");
	}
}

void LogUI::Tick()
{
	TickPanel();
	auto LogMessages = Log::GetMessages();

	if (LogPrompt->GetText() != LastCommand)
	{
		UpdateAutoComplete();
	}

	if (LogPrompt->GetIsEdited() && Input::IsKeyDown(Input::Key::TAB))
	{
		LogPrompt->SetText(AutoComplete->CompleteSelection(LogPrompt->GetText()));
		LogPrompt->Edit();
	}

	if (Input::IsKeyDown(Input::Key::UP))
	{
		if (!UpDownPressed)
		{
			AutoComplete->SelectionIndex--;
			UpdateAutoComplete();
		}
		UpDownPressed = true;
	}
	else if (Input::IsKeyDown(Input::Key::DOWN))
	{
		if (!UpDownPressed)
		{
			AutoComplete->SelectionIndex++;
			UpdateAutoComplete();
		}
		UpDownPressed = true;
	}
	else
	{
		UpDownPressed = false;
	}

	if (LogMessages.size() != PrevLogLength)
	{
		if (!LogTexts.empty())
		{
			LogTexts[LogTexts.size() - 1]->SetPadding(-0.001f, -0.001f, 0.001f, 0.001f);
		}

		for (; PrevLogLength < LogMessages.size(); PrevLogLength++)
		{
			std::string Text = LogMessages.at(PrevLogLength).Text;
			if (LogMessages.at(PrevLogLength).Amount >= 1)
			{
				Text.append(" (x" + std::to_string(LogMessages.at(PrevLogLength).Amount + 1) + ")");
			}
			LogTexts.push_back((new UIText(LogTextSize, LogMessages.at(PrevLogLength).Color, Text, EditorUI::MonoText)));

			LogScrollBox->AddChild(LogTexts.at(LogTexts.size() - 1)
				->SetWrapEnabled(true, 1.75f * LogScrollBox->GetUsedSize().X, UIBox::SizeMode::ScreenRelative)
				->SetPadding(-0.001f, PrevLogLength == LogMessages.size() - 1 ? 0.015f : -0.001f, 0.001f, 0.001f));
			PrevAmount = LogMessages.at(LogMessages.size() - 1).Amount;
		}

		if (!LogElements.empty())
		{
			for (auto& i : LogElements)
			{
				PrintUIElement(i);
			}
			LogElements.clear();
		}

		ResetScroll();
	}
	else if (!LogMessages.empty() && PrevAmount != LogMessages.at(LogMessages.size() - 1).Amount)
	{
		size_t Last = LogMessages.size() - 1;
		std::string Text = LogMessages.at(Last).Text;
		if (LogMessages.at(Last).Amount >= 1)
		{
			Text.append(" (x" + std::to_string(LogMessages.at(Last).Amount + 1) + ")");
		}
		LogTexts[Last]->SetText(Text);
		PrevAmount = LogMessages.at(Last).Amount;
	}
}
#endif