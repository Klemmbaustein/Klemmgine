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
	LogPromt->SetColor(UIColors[1] * 0.5);
	LogPromt->SetTryFill(true);
	LogScrollBox->SetMinSize((Scale - Vector2(0.2, 0.1)).Clamp(Vector2(0.7, 0.1), Vector2(1.4, 1)));
	LogScrollBox->SetMaxSize((Scale - Vector2(0.2, 0.1)).Clamp(Vector2(0.7, 0.1), Vector2(1.4, 1)));

}

LogUI::LogUI(Vector3* UIColors, Vector2 Position, Vector2 Scale) : EditorPanel(UIColors, Position, Scale, Vector2(0.8, 0.35), Vector2(2, 0.6))
{
	LogScrollBox = new UIScrollBox(false, 0, 0);
	LogScrollBox->Align = UIBox::E_REVERSE;
	LogPromt = new UITextField(true, 0, UIColors[1] * 0.5, this, 0, Editor::CurrentUI->EngineUIText);
	LogPromt->HintText = "Enter command here";
	TabBackground->AddChild((new UIBackground(false, 0, UIColors[1] * 0.99, 0))
		->SetBorder(UIBox::E_ROUNDED, 0.5)
		->AddChild(LogPromt
			->SetTextSize(0.45)
			->SetPadding(0)
			->SetTryFill(true))
		->AddChild(LogScrollBox
			->SetScrollSpeed(4)
			->SetPadding(0.01, 0, 0.01, 0.01)));

	UpdateLogBoxSize();
}

void LogUI::UpdateLayout()
{
	UpdateLogBoxSize();
	if (LogTexts.size())
	{
		float TextDifference = LogScrollBox->GetPosition().Y - LogTexts[LogTexts.size() - 1]->GetPosition().Y;
		LogScrollBox->GetScrollObject()->Percentage = std::max(TextDifference + 0.025, 0.0);
		LogScrollBox->SetMaxScroll(std::max(TextDifference + 0.025, 0.0) * 10);
	}
}

void LogUI::OnButtonClicked(int Index)
{
	if (Index == 0)
	{
		if (Input::IsKeyDown(SDLK_RETURN))
		{
			Log::Print("> " + LogPromt->GetText(), Vector3(0.3, 0.6, 1));
			Console::ExecuteConsoleCommand(LogPromt->GetText());
		}
		LogPromt->SetText("");
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
			LogTexts.push_back((new UIText(0.4, Log::Messages[i].Color, Text, Editor::CurrentUI->EngineUIText)));
			LogScrollBox->AddChild(LogTexts[LogTexts.size() - 1]
				->SetPadding(-0.003));
		}
		// If NewLogTexts is emtpy too, we skip calculating scroll related stuff.
		if (LogTexts.size())
		{
			// Update positions of everything first
			UIBox::DrawAllUIElements();

			float TextDifference = LogScrollBox->GetPosition().Y - LogTexts[LogTexts.size() - 1]->GetPosition().Y;
			LogScrollBox->GetScrollObject()->Percentage = std::max(TextDifference + 0.025, 0.0);
			LogScrollBox->SetMaxScroll(std::max(TextDifference + 0.025, 0.0) * 10);
			UIBox::RedrawUI();
		}
	}
}
#endif