#if EDITOR
#include "LogUI.h"
#include <UI/UIScrollBox.h>
#include <UI/UIText.h>
#include <Engine/Log.h>
#include <UI/EditorUI/EditorUI.h>
#include <UI/UITextField.h>
#include <Engine/Console.h>
#include <Engine/Input.h>
#include <UI/UIText.h>

static float LogTextSize = 0.38f;

void LogUI::UpdateLogBoxSize()
{
	LogPrompt->SetColor(EditorUI::UIColors[1] * 0.5f);
	LogScrollBox->SetMinSize((Scale - Vector2(0.06f, 0.1f)));
	LogScrollBox->SetMaxSize((Scale - Vector2(0.06f, 0.1f)));
}

LogUI::LogUI(EditorPanel* Parent) : EditorPanel(Parent, "Console")
{
	LogScrollBox = new UIScrollBox(UIBox::Orientation::Vertical, 0, true);
	LogPrompt = new UITextField(0, EditorUI::UIColors[1] * 0.5f, this, 0, EditorUI::MonoText);
	LogPrompt->HintText = "Console";
	PanelMainBackground->AddChild((new UIBackground(UIBox::Orientation::Vertical, 0, EditorUI::UIColors[1] * 0.99f, 0))
		->AddChild(LogScrollBox
			->SetScrollSpeed(4)
			->SetPadding(0, 0, 0.01f, 0))
		->AddChild(LogPrompt
			->SetTextSize(0.45f)
			->SetPadding(0)
			->SetTryFill(true)));

	UpdateLogBoxSize();
}

void LogUI::OnResized()
{
	UpdateLogBoxSize();
	if (LogTexts.size())
	{
		float TextDifference = LogScrollBox->GetPosition().Y - LogTexts.at(LogTexts.size() - 1)->GetPosition().Y;
		LogScrollBox->GetScrollObject()->Percentage = std::max(TextDifference + 0.05f, 0.0f);
		LogScrollBox->SetMaxScroll(std::max(TextDifference + 0.05f, 0.0f));

		for (UIText* i : LogTexts)
		{
			i->SetWrapEnabled(true, 1.75f * LogScrollBox->GetUsedSize().X, UIBox::SizeMode::ScreenRelative);
		}
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
	if (LogMessages.size() != PrevLogLength || (LogMessages.size() && PrevAmount != LogMessages.at(LogMessages.size() - 1).Amount))
	{
		PanelMainBackground->UpdateSelfAndChildren();
		PrevLogLength = LogMessages.size();
		float PrevPos = 0;
		if (LogMessages.size())
		{
			PrevAmount = LogMessages.at(LogMessages.size() - 1).Amount;
		}
		LogTexts.clear();
		LogScrollBox->DeleteChildren();
		for (size_t i = 0; i < LogMessages.size(); i++)
		{
			std::string Text = LogMessages.at(i).Text;
			if (LogMessages.at(i).Amount >= 1)
			{
				Text.append(" (x" + std::to_string(LogMessages.at(i).Amount + 1) + ")");
			}
			LogTexts.push_back((new UIText(LogTextSize, LogMessages.at(i).Color, Text, EditorUI::MonoText)));
			LogScrollBox->AddChild(LogTexts.at(LogTexts.size() - 1)
				->SetWrapEnabled(true, 1.75f * LogScrollBox->GetUsedSize().X, UIBox::SizeMode::ScreenRelative)
				->SetPadding(-0.001f));
		}
		// If NewLogTexts is emtpy too, we skip calculating scroll related stuff.
		if (LogTexts.size())
		{
			// Update positions of everything first
			UIBox::DrawAllUIElements();

			float TextDifference = LogScrollBox->GetPosition().Y - LogTexts.at(LogTexts.size() - 1)->GetPosition().Y;
			LogScrollBox->GetScrollObject()->Percentage = std::max(TextDifference + 0.025f, 0.0f);
			LogScrollBox->SetMaxScroll(std::max(TextDifference + 0.025f, 0.0f));
			UIBox::RedrawUI();
		}
	}
}
#endif