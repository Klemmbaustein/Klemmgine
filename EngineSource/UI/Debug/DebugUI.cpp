#if !EDITOR && !RELEASE
#include "DebugUI.h"
#include <UI/UIText.h>
#include <UI/UITextField.h>
#include <UI/UIBackground.h>

#include <Math/Collision/Collision.h>

#include <Engine/Application.h>
#include <Engine/Console.h>
#include <Engine/Input.h>
#include <Engine/Log.h>

DebugUI* DebugUI::CurrentDebugUI = nullptr;

DebugUI::DebugUI()
{
	CurrentDebugUI = this;
	Text = new TextRenderer();

	UIBox* DebugTextBackground = new UIBox(false, Vector2(-0.99, 0.7));
	DebugTextBackground->Align = UIBox::E_REVERSE;

	for (auto& i : DebugTexts)
	{
		i = new UIText(1, Vector3(1, 1, 0), "", Text);
		i->SetPadding(-0.005);
		DebugTextBackground->AddChild(i);
	}

	LogPrompt = new UITextField(true, -1, 0.1, this, 0, Text);
	LogPrompt->SetMinSize(Vector2(2, 0.06));
	LogPrompt->SetTextSize(0.6);
	LogBackground = new UIBackground(false, Vector2(-1, -0.94), 0.05, Vector2(2, 0.8));
	LogBackground->SetOpacity(0.9);
}

void DebugUI::Tick()
{
	if (StatsRedrawTimer >= 1)
	{
		DebugTexts[0]->SetText("FPS: " + std::to_string(FPS));
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
	if (Input::IsKeyDown(SDLK_RETURN) && !LogPrompt->GetIsEdited() && !IsEditingText)
	{
		IsEditingText = 5;
		LogPrompt->Edit();
	}
	if (!Input::IsKeyDown(SDLK_RETURN) && IsEditingText)
	{
		IsEditingText--;
	}

	LogPrompt->IsVisible = LogPrompt->GetIsEdited();
	LogBackground->IsVisible = LogPrompt->GetIsEdited();
	if (Log::Messages.size() != LogLength)
	{
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
		if (Input::IsKeyDown(SDLK_RETURN) && LogPrompt->GetText().size())
		{
			Log::Print("> " + LogPrompt->GetText(), Vector3(0.3, 0.6, 1));
			Console::ExecuteConsoleCommand(LogPrompt->GetText());
		}
		LogPrompt->SetText("");
	}
}
void DebugUI::GenerateLog()
{
	LogBackground->DeleteChildren();

	for (int64_t i = Log::Messages.size() - 1; i > std::max(0ll, (int64_t)Log::Messages.size() - 21); i--)
	{
		std::string str = Log::Messages[i].Text;
		if (Log::Messages[i].Amount >= 1)
		{
			str.append(" (x" + std::to_string(Log::Messages[i].Amount + 1) + ")");
		}
		LogBackground->AddChild((new UIText(0.6, Log::Messages[i].Color, str, Text))->SetPadding(0));
	}
}
#endif