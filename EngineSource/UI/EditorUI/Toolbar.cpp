#if EDITOR
#include "Toolbar.h"
#include <Engine/Log.h>
#include <UI/UIText.h>
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Subsystem/Scene.h>
#include <thread>
#include <UI/EditorUI/Viewport.h>
#include <Engine/Build/Build.h>
#include <filesystem>
#include <UI/EditorUI/SettingsPanel.h>
#include <UI/EditorUI/Popups/BakeMenu.h>
#include <Engine/Application.h>

#define MAX_CATEGORY_BUTTONS 16

Toolbar* Toolbar::ToolbarInstance = nullptr;

void Toolbar::GenerateButtons()
{
	ButtonsBox->DeleteChildren();
	ButtonsBox->SetMinSize(Scale);
	for (size_t i = 0; i < Buttons.size(); i++)
	{
		bool IsVisible = false;
		for (auto& btn : Buttons.at(i).Buttons)
		{
			if (btn.IsVisible)
			{
				IsVisible = true;
			}
		}
		if (!IsVisible)
		{
			continue;
		}

		for (size_t j = 0; j < Buttons[i].Buttons.size(); j++)
		{
			if (!Buttons[i].Buttons[j].IsVisible)
			{
				continue;
			}
			ButtonsBox->AddChild((new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2], this, (int)i * 16 + (int)j))
				->SetUseTexture(true, Buttons[i].Buttons[j].Texture)
				->SetSizeMode(UIBox::SizeMode::AspectRelative)
				->SetPadding(0, 0, 0.01f, 0.01f)
				->SetMinSize(0.04f));

			ButtonsBox->AddChild((new UIText(0.4f, EditorUI::UIColors[2], Buttons[i].Buttons[j].Name, EditorUI::Text))
				->SetPadding(0, 0, 0, 0.01f));
		}
		if (i != Buttons.size() - 1)
		{
			ButtonsBox->AddChild((new UIBackground(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2] * 0.5f, Vector2(2.0f / Graphics::WindowResolution.X, 0.04f)))
				->SetPadding(0.015f, 0.015f, 0.01f, 0.01f));
		}
	}
}

void Toolbar::RemoveButton(std::string Name)
{
	for (auto& cat : Buttons)
	{
		for (size_t i = 0; i < cat.Buttons.size(); i++)
		{
			if (cat.Buttons[i].Name == Name)
			{
				cat.Buttons.erase(cat.Buttons.begin() + i);
				GenerateButtons();
				return;
			}
		}
	}
}

void Toolbar::SetButtonVisibility(std::string Name, bool IsVisible)
{
	for (auto& cat : Buttons)
	{
		for (size_t i = 0; i < cat.Buttons.size(); i++)
		{
			if (cat.Buttons[i].Name == Name)
			{
				if (cat.Buttons[i].IsVisible != IsVisible)
				{
					cat.Buttons[i].IsVisible = IsVisible;
					GenerateButtons();
				}
				return;
			}
		}
	}
}


Toolbar::Toolbar(EditorPanel* Parent) : EditorPanel(Parent, "Toolbar")
{
	ToolbarInstance = this;
	RegisterNewButtonCategory(ButtonCategory("Scene", 
		{
			ButtonCategory::Button("Save", Application::EditorInstance->Textures[2], []() 
				{
					EditorUI::SaveCurrentScene();
				}),
			ButtonCategory::Button("Wireframe", Application::EditorInstance->Textures[1], []()
				{
					Log::Print("Toggled wireframe", Vector3(0.3f, 0.4f, 1));
					Graphics::IsWireframe = !Graphics::IsWireframe;
				}),
			ButtonCategory::Button("Bake", Application::EditorInstance->Textures[27], []() 
				{
					new BakeMenu();
				})
		}));
	RegisterNewButtonCategory(ButtonCategory("Project",
		{
			
			ButtonCategory::Button("Settings", Application::EditorInstance->Textures[20], []() {
					SettingsPanel::NewSettingsPanel();
				}),
			ButtonCategory::Button("Build", Application::EditorInstance->Textures[3], []() { new std::thread(Build::TryBuildProject, "GameBuild/"); }),
			ButtonCategory::Button("Run", Application::EditorInstance->Textures[21], []()
				{
					new std::thread([]() {
						EditorUI::LaunchInEditor();
					});
				})
		}));

#ifdef ENGINE_CSHARP
	RegisterNewButtonCategory(ButtonCategory("C#",
		{
			ButtonCategory::Button("Reload C#", Application::EditorInstance->Textures[12], []() { new std::thread(EditorUI::RebuildAssembly); }),
		}));
#endif

	ButtonsBox = new UIBox(UIBox::Orientation::Horizontal, 0);
	PanelMainBackground->AddChild(ButtonsBox
		->SetVerticalAlign(UIBox::Align::Centered)
		->SetPadding(0));

	GenerateButtons();
}

void Toolbar::OnResized()
{
	GenerateButtons();
}

void Toolbar::Tick()
{
}

void Toolbar::OnButtonClicked(int Index)
{
	HandlePanelButtons(Index);
	if (Index >= 0)
	{
		int Category = Index / MAX_CATEGORY_BUTTONS;
		int ButtonIndex = Index % MAX_CATEGORY_BUTTONS;
		if (Buttons.at(Category).Buttons.at(ButtonIndex).OnPressed)
		{
			Buttons[Category].Buttons[ButtonIndex].OnPressed();
		}
	}
}

void Toolbar::RegisterNewButtonCategory(ButtonCategory NewButtons)
{
	Buttons.push_back(NewButtons);
}
#endif