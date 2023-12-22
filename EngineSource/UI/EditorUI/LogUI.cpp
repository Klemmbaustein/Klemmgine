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
	LogPrompt->SetColor(UIColors[1] * 0.5f);
	LogScrollBox->SetMinSize((Scale - Vector2(0.2f, 0.1f)).Clamp(Vector2(0.7f, 0.1f), Vector2(1.4f, 1.0f)));
	LogScrollBox->SetMaxSize((Scale - Vector2(0.2f, 0.1f)).Clamp(Vector2(0.7f, 0.1f), Vector2(1.4f, 1.0f)));
	LogPrompt->SetMinSize(Vector2(LogScrollBox->GetMinSize().X, 0));
}

LogUI::LogUI(Vector3* UIColors, Vector2 Position, Vector2 Scale) : EditorPanel(UIColors, Position, Scale, Vector2(0.8f, 0.35f), Vector2(2, 0.6f))
{
	LogScrollBox = new UIScrollBox(false, 0, true);
	LogScrollBox->SetTryFill(true);
	LogPrompt = new UITextField(0, UIColors[1] * 0.5f, this, 0, Editor::CurrentUI->EngineUIText);
	LogPrompt->HintText = "Enter command here";
	TabBackground->AddChild((new UIBackground(false, 0, UIColors[1] * 0.99f, 0))
		->AddChild(LogScrollBox
			->SetScrollSpeed(4)
			->SetPadding(0, 0, 0.01f, 0))
		->AddChild(LogPrompt
			->SetTextSize(0.45f)
			->SetPadding(0)
			->SetTryFill(true)));

	UpdateLogBoxSize();
}

void LogUI::UpdateLayout()
{
	UpdateLogBoxSize();
	if (LogTexts.size())
	{
		float TextDifference = LogScrollBox->GetPosition().Y - LogTexts.at(LogTexts.size() - 1)->GetPosition().Y;
		LogScrollBox->GetScrollObject()->Percentage = std::max(TextDifference + 0.025f, 0.0f);
		LogScrollBox->SetMaxScroll(std::max(TextDifference + 0.025f, 0.0f));
	}
}

void LogUI::OnButtonClicked(int Index)
{
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
	UpdatePanel();
	auto LogMessages = Log::GetMessages();
	if (LogMessages.size() != PrevLogLength || (LogMessages.size() && PrevAmount != LogMessages.at(LogMessages.size() - 1).Amount))
	{
		TabBackground->UpdateSelfAndChildren();
		PrevLogLength = LogMessages.size();
		float PrevPos = 0;
		if (LogMessages.size())
		{
			PrevAmount = LogMessages.at(LogMessages.size() - 1).Amount;
		}
		LogScrollBox->DeleteChildren();
		for (size_t i = 0; i < LogMessages.size(); i++)
		{
			std::string Text = LogMessages.at(i).Text;
			if (LogMessages.at(i).Amount >= 1)
			{
				Text.append(" (x" + std::to_string(LogMessages.at(i).Amount + 1) + ")");
			}
			LogTexts.push_back((new UIText(LogTextSize, LogMessages.at(i).Color, Text, Editor::CurrentUI->EngineUIText)));
			LogScrollBox->AddChild(LogTexts.at(LogTexts.size() - 1)
				->SetWrapEnabled(true, 1.75f * LogScrollBox->GetUsedSize().X, UIBox::SizeMode::ScreenRelative)
				->SetPadding(-0.003f));
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