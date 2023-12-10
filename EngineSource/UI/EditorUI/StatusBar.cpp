#if EDITOR
#include "StatusBar.h"
#include <UI/UIText.h>
#include <UI/EditorUI/EditorUI.h>
#include <Engine/EngineProperties.h>
#include <Engine/OS.h>
#include <Engine/Application.h>
#include <Engine/Input.h>
#include <UI/EditorUI/Popups/AboutWindow.h>
#include <UI/EditorUI/Popups/DialogBox.h>
#include <UI/EditorUI/Popups/ClassCreator.h>
#include <UI/EditorUI/Popups/BakeMenu.h>
#include <thread>
#include "Viewport.h"

StatusBar* CurrentStatusBar = nullptr;

struct MenuBarEntry
{
	std::string Name;
	void(*Function)() = nullptr;
	bool Seperator = false;
};

struct MenuBarItem
{
	std::string Name;
	std::vector<MenuBarEntry> Entries;
};

static std::vector<MenuBarItem> MenuBarItems =
{
	MenuBarItem("File",
		{
			MenuBarEntry("New", nullptr),
			MenuBarEntry("Open", nullptr),
			MenuBarEntry("Save", []() { Editor::CurrentUI->SaveCurrentScene(); }, true),
			MenuBarEntry("Build Project", []() { new std::thread(Build::TryBuildProject, "Build/"); }, true),
			MenuBarEntry("Exit", []() { Application::Quit(); })
		}),
	MenuBarItem("Edit",
		{
			MenuBarEntry("Settings", []() { Viewport::ViewportInstance->OpenTab(6, "Settings.setting");}),
			MenuBarEntry("Run Project", []() {
			new std::thread([]() {
					EditorUI::LaunchInEditor();
				});
				}, true),
			MenuBarEntry("Bake Scene Shadows", []() { new BakeMenu(); }),
		}),
	MenuBarItem("View",
		{
			MenuBarEntry("Toggle Wireframe", []() {Graphics::IsWireframe = !Graphics::IsWireframe; }),
		}),
	MenuBarItem("C#",
		{
			MenuBarEntry("Open Solution", []() { OS::OpenFile(Build::GetProjectBuildName() + ".sln"); }),
			MenuBarEntry("Reload C# Assembly", []() { new std::thread(EditorUI::RebuildAndHotReload); }, true),
			MenuBarEntry("New Class", []() { new ClassCreator(); }),
		}),
	MenuBarItem("Help",
		{
			MenuBarEntry("Documentation", []() { new DialogBox("Documentation", 0, "Not implemented", {DialogBox::Answer("Ok", nullptr)}); }),
			MenuBarEntry("About", []() { new AboutWindow(); })
		}),
};
static std::vector<UIButton*> MenuBarButtons;

StatusBar::StatusBar(Vector3* Colors)
	: EditorPanel(Colors, Vector2(-1, 0.95f), Vector2(2, 0.05f), Vector2(2, 0.05f), Vector2(2, 0.05f))
{
	CurrentStatusBar = this;

	BarBoxes[0] = new UIBox(true, 0);
	BarBoxes[1] = new UIBox(true, 0);
	TabBackground->SetHorizontal(true);
	TabBackground->AddChild(BarBoxes[0]
		->SetPadding(0)
		->SetMinSize(Vector2(1, 0)));
	TabBackground->AddChild(BarBoxes[1]
		->SetPadding(0)
		->SetHorizontalAlign(UIBox::Align::Reverse)
		->SetMinSize(Vector2(1, 0)));

	TabBackground->SetColor(UIColors[0] * 0.75f);
	TabBackground->SetBorder(UIBox::BorderType::None, 0);

	std::string VersionText = "Engine v" + std::string(VERSION_STRING);
#if ENGINE_CSHARP
	VersionText.append("-C#");
#endif

	StatusText = new UIText(0.48f, UIColors[2], "FPS: ", Editor::CurrentUI->EngineUIText);
	BarBoxes[1]->AddChild(StatusText->SetPadding(0, 0, 0.01f, 0.025f));
	TabBackground->SetVerticalAlign(UIBox::Align::Centered);
	
	int Iterator = 0;
	for (const auto& i : MenuBarItems)
	{
		UIButton* NewButton = new UIButton(true, 0, UIColors[0] * 0.75f, this, Iterator++);
		BarBoxes[0]
			->AddChild(NewButton
				->SetPadding(0, 0, 0.005f, 0.005f)
				->AddChild((new UIText(0.45f, UIColors[2], i.Name, Editor::CurrentUI->EngineUIText))
					->SetPadding(0.005f)));
		MenuBarButtons.push_back(NewButton);
	}
}


bool StatusBar::IsHovered()
{
	return CurrentStatusBar->TabBackground->IsHovered()
		|| Vector3::NearlyEqual(Vector3(Input::MouseLocation, 0), Vector3(1), 0.02f);
}

void StatusBar::GenerateMenuBarDropdown(int ButtonIndex)
{
	if (MenuBarDropdown)
	{
		delete MenuBarDropdown;
		for (UIButton* i : MenuBarButtons)
		{
			i->SetColor(UIColors[0] * 0.75f);
		}
	}
	MenuBarDropdown = new UIBox(false, MenuBarButtons[ButtonIndex]->GetPosition() - Vector2(0, 0.5f));
	MenuBarDropdown->SetMinSize(Vector2(0.0f, 0.5f));
	MenuBarButtons[ButtonIndex]->SetColor(UIColors[0] * 1.5f);
	auto Background = new UIBackground(false, 0, Vector3::Lerp(UIColors[0], UIColors[2], 0.5f), 0);
	MenuBarDropdown->AddChild(Background
		->SetPadding(0));

	int it = 0;
	for (auto& i : MenuBarItems[ButtonIndex].Entries)
	{
		float PaddingSize = 2.0f / Graphics::WindowResolution.Y;
		bool Upper = MenuBarItems[ButtonIndex].Entries[0].Name == i.Name;
		bool Lower = i.Seperator || MenuBarItems[ButtonIndex].Entries[MenuBarItems[ButtonIndex].Entries.size() - 1].Name == i.Name;

		float Horizontal = 2.0f / Graphics::WindowResolution.X;

		Background->AddChild((new UIButton(true, 0, UIColors[0] * 1.5f, this, 100 + it++))
			->SetPadding(PaddingSize * (float)Upper, PaddingSize * (float)Lower, Horizontal, Horizontal)
			->SetMinSize(Vector2(0.2f, 0))
			->AddChild((new UIText(0.45f, UIColors[2], i.Name, Editor::CurrentUI->EngineUIText))
				->SetPadding(0.005f)));
	}
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
			+ ", Delta: " + std::to_string((int)(1.0f / DisplayedFPS * 1000))
			+ "ms, Mem: " + std::to_string(OS::GetMemUsage() / 1000ull / 1000ull) + "mb";
		StatusText->SetText(StatsText);
		FPSUpdateTimer = 0;
		DisplayedFPS = 0;
	}
	DisplayedFPS++;
	FPSUpdateTimer += Performance::DeltaTime;
	if ((Input::IsLMBDown && MenuBarDropdown && !MenuBarDropdown->IsHovered()) || Input::IsRMBDown)
	{
		for (UIButton* i : MenuBarButtons)
		{
			i->SetColor(UIColors[0] * 0.75f);
		}

		delete MenuBarDropdown;
		MenuBarDropdown = nullptr;
	}

}
void StatusBar::OnButtonClicked(int Index)
{
	if (Index < 10)
	{
		GenerateMenuBarDropdown(Index);
		Selected = Index;
	}
	else
	{
		if (MenuBarDropdown)
		{
			auto Function = MenuBarItems[Selected].Entries[Index - 100].Function;
			if (Function)
			{
				Function();
			}
			delete MenuBarDropdown;
			for (UIButton* i : MenuBarButtons)
			{
				i->SetColor(UIColors[0] * 0.75f);
			}
			MenuBarDropdown = nullptr;
		}
	}
}
#endif