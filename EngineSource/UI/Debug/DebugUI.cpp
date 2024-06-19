#if !EDITOR && !RELEASE && !SERVER
#include "DebugUI.h"
#include <UI/UIText.h>
#include <UI/UITextField.h>
#include <UI/UIBackground.h>

#include <Math/Collision/Collision.h>

#include <Engine/Application.h>
#include <Engine/Subsystem/Console.h>
#include <Engine/Log.h>

namespace Input
{
	extern bool BlockInputConsole;
}
using namespace Debug;

DebugUI* DebugUI::CurrentDebugUI = nullptr;


void DebugUI::UpdateAutoComplete()
{
	LastCommand = LogPrompt->GetText();

	AutoComplete->RenderToBox(CompleteBackground, AutoComplete->GetRecommendations(LastCommand));
}

DebugUI::DebugUI()
{
	CurrentDebugUI = this;
	Text = new TextRenderer();

	UIBox* DebugTextBackground = new UIBox(UIBox::Orientation::Vertical, Vector2(-0.985f, 0.695f));
	DebugTextBackground->SetVerticalAlign(UIBox::Align::Reverse);

	for (auto& i : DebugTexts)
	{
		i = new UIText(1, Vector3(1, 1, 0), "", Text);
		i->SetPadding(-0.005f);
		DebugTextBackground->AddChild(i);
	}

	LogPrompt = new UITextField(-1, 0.1f, this, 0, Text);
	LogPrompt->SetMinSize(Vector2(2, 0.06f));
	LogPrompt->SetTextSize(0.6f);
	LogBackground = new UIBackground(UIBox::Orientation::Vertical, Vector2(-1, -0.94f), 0.05f, Vector2(2, 0.8f));
	LogBackground->SetOpacity(0.9f);
	LogBackground->SetVerticalAlign(UIBox::Align::Default);

	AutoComplete = new ConsoleAutoComplete(Text, 0.6f);

	CompleteBackground = new UIBackground(UIBox::Orientation::Vertical, Vector2(-1, -0.94f), 0, Vector2(2, 0.0f));

	CompleteBackground->SetOpacity(0.95f);
}

bool DebugUI::ConsoleReadInput(Input::Key KeyCode)
{
	bool PrevBlockInput = false;
	std::swap<bool>(Input::BlockInput, PrevBlockInput);
	Input::BlockInputConsole = false;
	bool KeyVal = Input::IsKeyDown(KeyCode);
	std::swap(Input::BlockInput, PrevBlockInput);
	Input::BlockInputConsole = LogPrompt->GetIsEdited();
	return KeyVal;
}

void DebugUI::Tick()
{
	if (LogPrompt->GetText() != LastCommand)
	{
		UpdateAutoComplete();
	}

	if (LogPrompt->GetIsEdited() && ConsoleReadInput(Input::Key::TAB))
	{
		LogPrompt->SetText(AutoComplete->CompleteSelection(LogPrompt->GetText()));
		LogPrompt->Edit();
	}

	if (ConsoleReadInput(Input::Key::UP))
	{
		if (!UpDownPressed)
		{
			AutoComplete->SelectionIndex--;
			UpdateAutoComplete();
		}
		UpDownPressed = true;
	}
	else if (ConsoleReadInput(Input::Key::DOWN))
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


	auto LogMessages = Log::GetMessages();
	if (StatsRedrawTimer >= 1)
	{
		DebugTexts[0]->SetText("FPS: " + std::to_string(FPS - 1));
		DebugTexts[1]->SetText("Delta: " + std::to_string(1000 / FPS) + "ms");

		std::string DeltaString;
		DeltaString.append(std::to_string((int)(Application::LogicTime / Stats::DeltaTime * 100.f)) + "% Log ");
		DeltaString.append(std::to_string((int)(Application::RenderTime / Stats::DeltaTime * 100.f)) + "% Rend ");
		DeltaString.append(std::to_string((int)(Application::SyncTime / Stats::DeltaTime * 100.f)) + "% Buf");

		DebugTexts[2]->SetText(DeltaString);
		DebugTexts[3]->SetText("DrawCalls: " + std::to_string(Stats::DrawCalls));
		StatsRedrawTimer = 0;
		FPS = 0;
	}
	else
	{
		StatsRedrawTimer += Stats::DeltaTime;
	}
	FPS++;
	if (ConsoleReadInput(Input::Key::RETURN) && !LogPrompt->GetIsEdited() && !IsEditingText)
	{
		IsEditingText = 5;
		LogPrompt->Edit();
	}
	if (!ConsoleReadInput(Input::Key::RETURN) && IsEditingText)
	{
		IsEditingText--;
	}

	LogPrompt->IsVisible = LogPrompt->GetIsEdited();
	LogBackground->IsVisible = LogPrompt->GetIsEdited();
	if (LogMessages.size() != LogLength || (!LogMessages.empty() && LastLogMessageAmount != LogMessages[LogMessages.size() - 1].Amount))
	{
		if (!LogMessages.empty())
		{
			LastLogMessageAmount = LogMessages[LogMessages.size() - 1].Amount;
		}
		LogLength = LogMessages.size();
		GenerateLog();
	}
}

DebugUI::~DebugUI()
{
	CurrentDebugUI = nullptr;
	delete AutoComplete;
	delete CompleteBackground;
	delete Text;
}
void DebugUI::OnButtonClicked(int Index)
{
	if (Index == 0)
	{
		if (ConsoleReadInput(Input::Key::RETURN) && LogPrompt->GetText().size())
		{
			Log::Print("> " + LogPrompt->GetText(), Vector3(0.3f, 0.6f, 1));
			Console::ExecuteConsoleCommand(LogPrompt->GetText());
		}
		LogPrompt->SetText("");
	}
}
void DebugUI::GenerateLog()
{
	auto LogMessages = Log::GetMessages();

	LogBackground->DeleteChildren();

	if (LogMessages.empty())
	{
		return;
	}

	for (int64_t i = LogMessages.size() - 1; i >= std::max((int64_t)0, (int64_t)LogMessages.size() - 23); i--)
	{
		std::string str = LogMessages[i].Text;
		if (LogMessages[i].Amount >= 1)
		{
			str.append(" (x" + std::to_string(LogMessages[i].Amount + 1) + ")");
		}
		LogBackground->AddChild((new UIText(0.5f, LogMessages[i].Color, str, Text))->SetPadding(0));
	}
}
#endif