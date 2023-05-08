#if EDITOR
#include "StatusBar.h"
#include <UI/UIText.h>
#include <UI/EditorUI/EditorUI.h>
#include <Engine/EngineProperties.h>
#include <Engine/OS.h>

StatusBar::StatusBar(Vector3* Colors) : EditorPanel(Colors, Vector2(-1, 0.95), Vector2(2, 0.05), Vector2(2, 0.05), Vector2(2, 0.05))
{
	TabBackground->SetColor(Colors[1]);
	Texts[0] = new UIText(0.6, UIColors[2], "FPS: ?", Editor::CurrentUI->EngineUIText);
	TabBackground->AddChild(Texts[0]->SetPadding(0.005, 0.005, 0.01, 0.025));
	Texts[1] = new UIText(0.4, UIColors[2], ProjectName, Editor::CurrentUI->EngineUIText);
	TabBackground->AddChild(Texts[1]->SetPadding(0, 0.01, 0.01, 0));
	Texts[2] = new UIText(0.4, UIColors[2], std::string(VERSION_STRING) + "-Editor", Editor::CurrentUI->EngineUIText);
	TabBackground->AddChild(Texts[2]->SetPadding(0, 0.01, 0.01, 0));
	auto HelpButton = new UIButton(true, 0, UIColors[0], this, 0);
	HelpButton->Align = UIBox::E_CENTERED;
	TabBackground->AddChild(HelpButton
		->SetPadding(0.01, 0.01, 0.02, 0.01)
		->SetMinSize(0.03)
		->SetBorder(UIBox::E_ROUNDED, 0.6)
		->SetSizeMode(UIBox::E_PIXEL_RELATIVE)
		->AddChild((new UIText(0.4, UIColors[2], "?", Editor::CurrentUI->EngineUIText))
			->SetPadding(0.001)));
}

void StatusBar::UpdateLayout()
{
}

void StatusBar::Tick()
{
	UpdatePanel();

	// Measure FPS like this instead of calculating it from the DeltaTime
	// so we don't have to redraw the FPS counter every time the frame time changes.
	if (FPSUpdateTimer >= 1)
	{
		Texts[0]->SetText("FPS: " + std::to_string(DisplayedFPS));
		FPSUpdateTimer = 0;
		DisplayedFPS = 0;
	}
	DisplayedFPS++;
	FPSUpdateTimer += Performance::DeltaTime;
}
void StatusBar::OnButtonClicked(int Index)
{
	OS::OpenFile("../../docs/docs/index.html");
}
#endif