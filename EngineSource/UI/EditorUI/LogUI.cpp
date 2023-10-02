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
	LogScrollBox->SetAlign(UIBox::Align::Reverse);
	LogPrompt = new UITextField(true, 0, UIColors[1] * 0.5f, this, 0, Editor::CurrentUI->EngineUIText);
	LogPrompt->HintText = "Enter command here";
	TabBackground->AddChild((new UIBackground(false, 0, UIColors[1] * 0.99f, 0))
		->AddChild(LogPrompt
			->SetTextSize(0.45f)
			->SetPadding(0)
			->SetTryFill(true))
		->AddChild(LogScrollBox
			->SetScrollSpeed(4)
			->SetPadding(0, 0, 0.01f, 0)));

	UpdateLogBoxSize();
}

void LogUI::UpdateLayout()
{
	UpdateLogBoxSize();
	if (LogTexts.size())
	{
		float TextDifference = LogScrollBox->GetPosition().Y - LogTexts[LogTexts.size() - 1]->GetPosition().Y;
		LogScrollBox->GetScrollObject()->Percentage = std::max(TextDifference + 0.025f, 0.0f);
		LogScrollBox->SetMaxScroll(std::max(TextDifference + 0.025f, 0.0f));
	}
}

void LogUI::OnButtonClicked(int Index)
{
	if (Index == 0)
	{
		if (Input::IsKeyDown(SDLK_RETURN))
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
	if (Log::Messages.size() != PrevLogLength || (Log::Messages.size() && PrevAmount != Log::Messages[Log::Messages.size() - 1].Amount))
	{
		PrevLogLength = Log::Messages.size();
		float PrevPos = 0;
		if (Log::Messages.size())
		{
			PrevAmount = Log::Messages[Log::Messages.size() - 1].Amount;
		}
		LogScrollBox->DeleteChildren();
		for (size_t i = 0; i < Log::Messages.size(); i++)
		{
			std::string Text = Log::Messages[i].Text;
			if (Log::Messages[i].Amount >= 1)
			{
				Text.append(" (x" + std::to_string(Log::Messages[i].Amount + 1) + ")");
			}
			LogTexts.push_back((new UIText(0.38f, Log::Messages[i].Color, Text, Editor::CurrentUI->EngineUIText)));
			LogScrollBox->AddChild(LogTexts[LogTexts.size() - 1]
				->SetPadding(-0.003f));
		}
		// If NewLogTexts is emtpy too, we skip calculating scroll related stuff.
		if (LogTexts.size())
		{
			// Update positions of everything first
			UIBox::DrawAllUIElements();

			float TextDifference = LogScrollBox->GetPosition().Y - LogTexts[LogTexts.size() - 1]->GetPosition().Y;
			LogScrollBox->GetScrollObject()->Percentage = std::max(TextDifference + 0.025f, 0.0f);
			LogScrollBox->SetMaxScroll(std::max(TextDifference + 0.025f, 0.0f));
			UIBox::RedrawUI();
		}
	}
}
#endif