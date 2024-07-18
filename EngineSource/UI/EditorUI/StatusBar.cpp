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
#include <Engine/Subsystem/BackgroundTask.h>
#include "SettingsPanel.h"
#include <Math/Collision/CollisionVisualize.h>
#include <Engine/Utility/FileUtility.h>
#include <Engine/Subsystem/CSharpInterop.h>
#include "SerializePanel.h"

StatusBar* CurrentStatusBar = nullptr;

struct MenuBarEntry
{
	std::string Name;
	void(*Function)() = nullptr;
	bool Separator = false;
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
			MenuBarEntry("Open", []() 
				{ 
					std::string File = OS::ShowOpenFileDialog();
					if (FileUtil::GetExtension(File) != "jscn")
					{
						Log::Print("Can only open scene files.");
						return;
					}
					Application::EditorInstance->OpenScene(File);
				}),
			MenuBarEntry("Save", []() { Application::EditorInstance->SaveCurrentScene(); }, true),
			MenuBarEntry("Build Project", []() { new BackgroundTask([]() {Build::TryBuildProject("GameBuild/"); }); }, true),
			MenuBarEntry("Exit", []() { Application::Quit(); })
		}),
	MenuBarItem("Edit",
		{
			MenuBarEntry("Settings", []() {	SettingsPanel::NewSettingsPanel(); }),
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
			MenuBarEntry("Toggle Fullbright", []() {Graphics::RenderFullBright = !Graphics::RenderFullBright; }),
			MenuBarEntry("Toggle Collision View", []() {
					if (!CollisionVisualize::GetIsActive())
					{
						CollisionVisualize::Activate();
					}
					else
					{
						CollisionVisualize::Deactivate();
					}
				}
			),
		}),
	MenuBarItem("Window",
		{
			MenuBarEntry("Save Layout", []()
				{
					SaveData s = SaveData(Editor::SerializePanel::GetLayoutPrefFilePath(), "pref", false, true);
					Log::Print("Saving layout to '" + Editor::SerializePanel::GetLayoutPrefFilePath() + ".pref'");
					s.SetField(Editor::SerializePanel::SerializeLayout(EditorUI::RootPanel));
				}
			),
			MenuBarEntry("Load Layout", []()
				{
					SaveData s = SaveData(Editor::SerializePanel::GetLayoutPrefFilePath(), "pref", false, false);
					Log::Print("Loading layout from '" + Editor::SerializePanel::GetLayoutPrefFilePath() + ".pref'");
					EditorUI::LoadPanelLayout(Editor::SerializePanel::DeSerializeLayout(s.GetField("root")));
				}
			),
			MenuBarEntry("Load Default Layout", []()
				{
					EditorUI::LoadDefaultLayout();
				}
			),
		}),
#ifdef ENGINE_CSHARP
	MenuBarItem("C#",
		{
			MenuBarEntry("Open Solution", []() { OS::OpenFile(Build::GetProjectBuildName() + ".sln"); }),
			MenuBarEntry("Rebuild C# Assembly", []() { new BackgroundTask(EditorUI::RebuildAssembly); }, true),
			MenuBarEntry("New Class", []() { new ClassCreator(); }),
		}),
#endif
	MenuBarItem("Help",
		{
			MenuBarEntry("Documentation", []() { OS::OpenFile(Application::GetEditorPath() + "/Docs/html/index.html"); }),
			MenuBarEntry("About", []() { new AboutWindow(); })
		}),
};
static std::vector<UIButton*> MenuBarButtons;

StatusBar::StatusBar()
{
	CurrentStatusBar = this;
	BarBoxes[0] = new UIBox(UIBox::Orientation::Horizontal, 0);
	BarBoxes[1] = new UIBox(UIBox::Orientation::Horizontal, 0);
	StatusBackground = new UIBackground(UIBox::Orientation::Horizontal, Vector2(-1, 0.95f), 1, Vector2(2, 0.05f));
	StatusBackground->AddChild(BarBoxes[0]
		->SetMinSize(Vector2(1, 0)));
	StatusBackground->AddChild(BarBoxes[1]
		->SetHorizontalAlign(UIBox::Align::Reverse)
		->SetMinSize(Vector2(1, 0)));

	StatusBackground->SetBorder(UIBox::BorderType::None, 0);

	StatusText = new UIText(0.48f, EditorUI::UIColors[2], "FPS: ", EditorUI::Text);
	BarBoxes[1]->AddChild(StatusText->SetPadding(0, 0, 0.01f, 0.01f));
	StatusBackground->SetVerticalAlign(UIBox::Align::Centered);
	
	int Iterator = 0;
	for (const auto& i : MenuBarItems)
	{
#ifdef ENGINE_CSHARP
		if (i.Name != "C#" || CSharpInterop::GetUseCSharp())
#else
		if (true)
#endif
		{
			UIButton* NewButton = new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[0] * 0.75f, this, Iterator);
			BarBoxes[0]
				->AddChild(NewButton
					->SetPadding(0, 0, 0.005f, 0.005f)
					->AddChild((new UIText(0.45f, EditorUI::UIColors[2], i.Name, EditorUI::Text))
						->SetPadding(0.005f)));
			MenuBarButtons.push_back(NewButton);
		}
		else
		{
			MenuBarButtons.push_back(nullptr);
		}
		Iterator++;
	}
}

void StatusBar::GenerateMenuBarDropdown(int ButtonIndex)
{
	if (MenuBarDropdown)
	{
		delete MenuBarDropdown;
		for (UIButton* i : MenuBarButtons)
		{
			i->SetColor(EditorUI::UIColors[0] * 0.75f);
		}
	}
	MenuBarDropdown = new UIBox(UIBox::Orientation::Vertical, MenuBarButtons[ButtonIndex]->GetPosition() - Vector2(0, 0.5f));
	MenuBarDropdown->SetMinSize(Vector2(0.0f, 0.5f));
	MenuBarButtons[ButtonIndex]->SetColor(EditorUI::UIColors[0] * 1.5f);
	auto Background = new UIBackground(UIBox::Orientation::Vertical, 0, Vector3::Lerp(EditorUI::UIColors[0], EditorUI::UIColors[2], 0.5f), 0);
	MenuBarDropdown->AddChild(Background);
	int it = 0;
	for (auto& i : MenuBarItems[ButtonIndex].Entries)
	{
		float PaddingSize = 2.0f / Graphics::WindowResolution.Y;
		bool Upper = MenuBarItems[ButtonIndex].Entries[0].Name == i.Name;
		bool Lower = i.Separator || MenuBarItems[ButtonIndex].Entries[MenuBarItems[ButtonIndex].Entries.size() - 1].Name == i.Name;

		float Horizontal = 2.0f / Graphics::WindowResolution.X;

		Background->AddChild((new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[0] * 1.5f, this, 100 + it++))
			->SetPadding(PaddingSize * (float)Upper, PaddingSize * (float)Lower, Horizontal, Horizontal)
			->SetMinSize(Vector2(0.2f, 0))
			->AddChild((new UIText(0.45f, EditorUI::UIColors[2], i.Name, EditorUI::Text))
				->SetPadding(0.005f)));
	}
}

void StatusBar::Tick()
{
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
	if (StatusBackground->GetColor() != EditorUI::UIColors[0] * 0.75f)
	{
		for (UIButton* i : MenuBarButtons)
		{
			if (i != nullptr)
			{
				i->SetColor(EditorUI::UIColors[0] * 0.75f);
			}
		}
	}
	StatusBackground->SetColor(EditorUI::UIColors[0] * 0.75f);
	DisplayedFPS++;
	FPSUpdateTimer += Stats::DeltaTime;
	if ((Input::IsLMBDown && MenuBarDropdown && !MenuBarDropdown->IsHovered()) || Input::IsRMBDown)
	{
		for (UIButton* i : MenuBarButtons)
		{
			if (i != nullptr)
			{
				i->SetColor(EditorUI::UIColors[0] * 0.75f);
			}
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
				if (i != nullptr)
				{
					i->SetColor(EditorUI::UIColors[0] * 0.75f);
				}
			}
			MenuBarDropdown = nullptr;
		}
	}
}
#endif