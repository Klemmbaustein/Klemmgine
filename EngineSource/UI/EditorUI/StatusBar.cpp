#if EDITOR
#include "StatusBar.h"
#include <UI/UIText.h>
#include <UI/EditorUI/EditorUI.h>
#include <Engine/EngineProperties.h>

StatusBar::StatusBar(Vector3* Colors) : EditorTab(Colors, Vector2(-1, 0.95), Vector2(2, 0.05), Vector2(2, 0.05), Vector2(2, 0.05))
{
	TabBackground->SetColor(Colors[1]);
	Texts[0] = new UIText(0.6, UIColors[2] * 0.9, "FPS: ?", Editor::CurrentUI->EngineUIText);
	TabBackground->AddChild(Texts[0]->SetPadding(0, -0.01, 0.01, 0));
	Texts[1] = new UIText(0.4, UIColors[2] * 0.7, ProjectName, Editor::CurrentUI->EngineUIText);
	TabBackground->AddChild(Texts[1]->SetPadding(0, 0.01, 0.01, 0));
	Texts[2] = new UIText(0.4, UIColors[2] * 0.7, std::string(VERSION_STRING) + "-Editor", Editor::CurrentUI->EngineUIText);
	TabBackground->AddChild(Texts[2]->SetPadding(0, 0.01, 0.01, 0));
}

void StatusBar::Save()
{
}

void StatusBar::Load(std::string File)
{
}

void StatusBar::UpdateLayout()
{
}

void StatusBar::Tick()
{
	UpdateTab();
	if (FPSUpdateTimer >= 1)
	{
		Texts[0]->SetText("FPS: " + std::to_string(DisplayedFPS));
		FPSUpdateTimer = 0;
		DisplayedFPS = 0;
	}
	DisplayedFPS++;
	FPSUpdateTimer += Performance::DeltaTime;
}
#endif