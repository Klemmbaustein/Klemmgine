#if EDITOR
#include "StatusBar.h"
#include <UI/UIText.h>
#include <UI/EditorUI/EditorUI.h>
#include <Engine/EngineProperties.h>
#include <Engine/OS.h>
#include <Engine/Application.h>
#include <Engine/Input.h>

StatusBar* CurrentStatusBar = nullptr;

StatusBar::StatusBar(Vector3* Colors)
	: EditorPanel(Colors, Vector2(-1, 0.95f), Vector2(2, 0.05f), Vector2(2, 0.05f), Vector2(2, 0.05f))
{
	CurrentStatusBar = this;
	TabBackground->SetColor(UIColors[0] * 0.75f);
	TabBackground->SetBorder(UIBox::BorderType::None, 0);

	std::string VersionText = "Engine v" + std::string(VERSION_STRING);
#if ENGINE_CSHARP
	VersionText.append("-C#");
#endif

	Texts[2] = new UIText(0.5f, UIColors[2], "FPS: ", Editor::CurrentUI->EngineUIText);
	TabBackground->AddChild(Texts[2]->SetPadding(0, 0, 0.01f, 0.025f));
	TabBackground->SetVerticalAlign(UIBox::Align::Centered);
	WindowButtonBox = (new UIBox(true, Vector2(0.75f, 0.95f)))
		->SetMinSize(Vector2(0.25f, 0.05f));
	WindowButtonBox->SetHorizontalAlign(UIBox::Align::Centered);
}


bool StatusBar::IsHovered()
{
	return CurrentStatusBar->TabBackground->IsHovered()
		&& CurrentStatusBar->WindowButtonBox 
		&& !CurrentStatusBar->WindowButtonBox->IsHovered() 
		|| Vector3::NearlyEqual(Vector3(Input::MouseLocation, 0), Vector3(1), 0.02f);
}

void StatusBar::UpdateLayout()
{
	std::vector<int> Buttons;
	TabBackground->SetColor(UIColors[0] * 0.75);
}

void StatusBar::Tick()
{
	UpdatePanel();

	// Measure FPS like this instead of calculating it from the DeltaTime
	// so we don't have to redraw the FPS counter every time the frame time changes.
	if (FPSUpdateTimer >= 1)
	{
		std::string StatsText = "FPS: " + std::to_string(DisplayedFPS)
			+ "     Delta: " + std::to_string((int)(1.0f / DisplayedFPS * 1000))
			+ "ms     Memory used: " + std::to_string(OS::GetMemUsage() / 1000ull / 1000ull) + "mb";
		Texts[2]->SetText(StatsText);
		FPSUpdateTimer = 0;
		DisplayedFPS = 0;
	}
	DisplayedFPS++;
	FPSUpdateTimer += Performance::DeltaTime;

}
void StatusBar::OnButtonClicked(int Index)
{
	switch (Index)
	{
	case -1:
		OS::OpenFile("../../docs/docs/index.html");
		break;
	case 0:
		Application::Quit();
		break;
	case 1:
	case 2:
		Application::SetFullScreen(!Application::GetFullScreen());
		break;
	case 3:
		Application::Minimize();
		break;
	default:
		break;
	}
}
#endif