#if EDITOR
#include "StatusBar.h"
#include <UI/UIText.h>
#include <UI/EditorUI/EditorUI.h>
#include <Engine/EngineProperties.h>
#include <Engine/OS.h>
#include <Engine/Application.h>
#include <Engine/Input.h>

StatusBar* CurrentStatusBar = nullptr;

StatusBar::StatusBar(Vector3* Colors) : EditorPanel(Colors, Vector2(-1, 0.95), Vector2(2, 0.05), Vector2(2, 0.05), Vector2(2, 0.05))
{
	CurrentStatusBar = this;
	TabBackground->SetColor(UIColors[0] * 0.75);
	TabBackground->SetBorder(UIBox::E_NONE, 0);

	Texts[0] = new UIText(0.6, UIColors[2], "FPS: ?", Editor::CurrentUI->EngineUIText);
	TabBackground->AddChild(Texts[0]->SetPadding(0.005, 0.005, 0.01, 0.025));

	Texts[1] = new UIText(0.4, UIColors[2], ProjectName, Editor::CurrentUI->EngineUIText);
	TabBackground->AddChild(Texts[1]->SetPadding(0, 0.01, 0.01, 0));

	std::string VersionText = std::string(VERSION_STRING) + "-Editor";
#ifdef ENGINE_CSHARP
	VersionText.append("-C#");
#endif
	Texts[2] = new UIText(0.45, UIColors[2], VersionText, Editor::CurrentUI->EngineUIText);
	TabBackground->AddChild(Texts[2]->SetPadding(0, 0.01, 0.01, 0));

	WindowButtonBox = (new UIBox(true, Vector2(0.75, 0.95)))
		->SetMinSize(Vector2(0.25, 0.05));
	WindowButtonBox->Align = UIBox::E_REVERSE;
	UpdateLayout();
}

void StatusBar::GenerateWindowButtons(std::vector<int> ButtonIndices)
{
	WindowButtonBox->DeleteChildren();
	for (int i : ButtonIndices)
	{
		WindowButtonBox->AddChild((new UIButton(true, 0, UIColors[0] * 0.75, this, i))
			->SetMinSize(0.05)
			->SetPadding(0)
			->SetSizeMode(UIBox::E_PIXEL_RELATIVE)
			->AddChild((new UIBackground(true, 0, UIColors[2], Vector2(0.03)))
				->SetUseTexture(true, Editor::CurrentUI->Textures[23ull + (size_t)i])
				->SetSizeMode(UIBox::E_PIXEL_RELATIVE)
				->SetPadding(0.01)));
	}
}

bool StatusBar::IsHovered()
{
	return CurrentStatusBar->TabBackground->IsHovered()
		&& CurrentStatusBar->WindowButtonBox 
		&& !CurrentStatusBar->WindowButtonBox->IsHovered() 
		|| Vector3::NearlyEqual(Vector3(Input::MouseLocation, 0), Vector3(1), 0.02);
}

void StatusBar::UpdateLayout()
{
	std::vector<int> Buttons;
	if (Application::GetFullScreen())
	{
		Buttons = {0, 2, 3};
	}
	else
	{
		Buttons = { 0, 1, 3 };
	}
	GenerateWindowButtons(Buttons);
	TabBackground->SetColor(UIColors[0] * 0.75);
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