#if !EDITOR && !RELEASE
#include "DebugUI.h"
#include <UI/UIText.h>
#include <UI/UITextField.h>
#include <UI/UIBackground.h>

#include <Math/Collision/Collision.h>

#include <Engine/Application.h>
#include <Engine/Console.h>
#include <Engine/Log.h>

namespace Input
{
	extern bool BlockInputConsole;
}

DebugUI* DebugUI::CurrentDebugUI = nullptr;

DebugUI::DebugUI()
{
	CurrentDebugUI = this;
	Text = new TextRenderer();

	UIBox* DebugTextBackground = new UIBox(false, Vector2(-0.99f, 0.7f));
	DebugTextBackground->SetAlign(UIBox::Align::Reverse);

	for (auto& i : DebugTexts)
	{
		i = new UIText(1, Vector3(1, 1, 0), "", Text);
		i->SetPadding(-0.005f);
		DebugTextBackground->AddChild(i);
	}

	LogPrompt = new UITextField(true, -1, 0.1f, this, 0, Text);
	LogPrompt->SetMinSize(Vector2(2, 0.06f));
	LogPrompt->SetTextSize(0.6f);
	LogBackground = new UIBackground(false, Vector2(-1, -0.94f), 0.05f, Vector2(2, 0.8f));
	LogBackground->SetOpacity(0.9f);
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
	if (StatsRedrawTimer >= 1)
	{
		DebugTexts[0]->SetText("FPS: " + std::to_string(FPS - 1));
		DebugTexts[1]->SetText("Delta: " + std::to_string(1000 / FPS) + "ms");

		std::string DeltaString;
		DeltaString.append(std::to_string((int)(Application::LogicTime / Performance::DeltaTime * 100.f)) + "% Log ");
		DeltaString.append(std::to_string((int)(Application::RenderTime / Performance::DeltaTime * 100.f)) + "% Rend ");
		DeltaString.append(std::to_string((int)(Application::SyncTime / Performance::DeltaTime * 100.f)) + "% Buf");

		DebugTexts[2]->SetText(DeltaString);
		DebugTexts[3]->SetText("DrawCalls: " + std::to_string(Performance::DrawCalls));
		DebugTexts[4]->SetText("CollisonMeshes: " + std::to_string(Collision::CollisionBoxes.size()));
		StatsRedrawTimer = 0;
		FPS = 0;
	}
	else
	{
		StatsRedrawTimer += Performance::DeltaTime;
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
	if (Log::Messages.size() != LogLength || (!Log::Messages.empty() && LastLogMessageAmount != Log::Messages[Log::Messages.size() - 1].Amount))
	{
		if (!Log::Messages.empty())
		{
			LastLogMessageAmount = Log::Messages[Log::Messages.size() - 1].Amount;
		}
		LogLength = Log::Messages.size();
		GenerateLog();
	}
}

DebugUI::~DebugUI()
{
	CurrentDebugUI = nullptr;
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
	LogBackground->DeleteChildren();

	for (int64_t i = Log::Messages.size() - 1; i >= std::max(0ll, (int64_t)Log::Messages.size() - 21); i--)
	{
		std::string str = Log::Messages[i].Text;
		if (Log::Messages[i].Amount >= 1)
		{
			str.append(" (x" + std::to_string(Log::Messages[i].Amount + 1) + ")");
		}
		LogBackground->AddChild((new UIText(0.6f, Log::Messages[i].Color, str, Text))->SetPadding(0));
	}
}
#endif