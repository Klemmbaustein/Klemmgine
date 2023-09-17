#if EDITOR
#include "Toolbar.h"
#include <Engine/Log.h>
#include <UI/UIText.h>
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Scene.h>
#include <thread>
#include <UI/EditorUI/Viewport.h>
#include <Engine/Build/Build.h>
#include <CSharp/CSharpInterop.h>
#include <filesystem>
#include <UI/EditorUI/Popups/BakeMenu.h>

#define MAX_CATEGORY_BUTTONS 16

Toolbar* Toolbar::ToolbarInstance = nullptr;

void Toolbar::GenerateButtons()
{
	TabBackground->DeleteChildren();
	for (size_t i = 0; i < Buttons.size(); i++)
	{
		bool IsVisible = false;
		for (auto& btn : Buttons[i].Buttons)
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
		TabBackground->AddChild((new UIBackground(true, 0, UIColors[2] * 0.5f, Vector2(0.001f, 0.2f)))
			->SetPadding(0.01f, 0.01f, 0.02f, 0.01f));
		auto Elem = new UIBox(false, 0);
		Elem->SetAlign(UIBox::Align::Reverse);
		Elem->SetPadding(0.0f, 0.0f, 0, 0);
		Elem->AddChild((new UIText(0.4f, UIColors[2] * 0.7f, Buttons[i].Name, Editor::CurrentUI->EngineUIText))->SetPadding(0));
		TabBackground->AddChild(Elem);

		UIBox* ButtonBackground = new UIBox(true, 0);
		Elem->AddChild(ButtonBackground);
		ButtonBackground->SetPadding(0);

		size_t j = 0;
		for (auto& btn : Buttons[i].Buttons)
		{
			if (!btn.IsVisible)
			{
				j++;
				continue;
			}
			ButtonBackground->AddChild((new UIBackground(false, 0, UIColors[3], Vector2(0.1f)))
				->SetBorder(UIBox::BorderType::Rounded, 0.5f)
				->SetSizeMode(UIBox::SizeMode::PixelRelative)
				->AddChild((new UIText(0.4f, UIColors[2], btn.Name, Editor::CurrentUI->EngineUIText))
					->SetPadding(0.005f))
				->AddChild((new UIButton(true, 0, 1, this, (int)i * MAX_CATEGORY_BUTTONS + (int)j))
					->SetUseTexture(true, btn.Texture)
					->SetMinSize(0.075f)
					->SetPadding(0.015f)
					->SetSizeMode(UIBox::SizeMode::PixelRelative)));
			j++;
		}
	}
	TabBackground->AddChild((new UIBackground(true, 0, UIColors[2] * 0.5f, Vector2(0.001f, 0.2f)))
		->SetPadding(0.01f, 0.01f, 0.02f, 0.01f));
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


Toolbar::Toolbar(Vector3* Colors, Vector2 Position, Vector2 Scale) : EditorPanel(Colors, Position, Scale, Vector2(0.8f, 0.22f), Vector2(2, 0.45f))
{
	ToolbarInstance = this;
	RegisterNewButtonCategory(ButtonCategory("Scene", 
		{
			ButtonCategory::Button("Save", Editor::CurrentUI->Textures[2], []() {
					EditorUI::SaveCurrentScene();
				}),
			ButtonCategory::Button("Wireframe", Editor::CurrentUI->Textures[1], []() {
					Log::Print("Toggled wireframe", Vector3(0.3f, 0.4f, 1));
					Graphics::IsWireframe = !Graphics::IsWireframe;
				}),
			ButtonCategory::Button("Bake", Editor::CurrentUI->Textures[3], []() {
					new BakeMenu();
				})
		}));
	RegisterNewButtonCategory(ButtonCategory("Project",
		{
			
			ButtonCategory::Button("Settings", Editor::CurrentUI->Textures[20], []() { Viewport::ViewportInstance->OpenTab(6, "Settings.setting"); }),
			ButtonCategory::Button("Build", Editor::CurrentUI->Textures[3], []() { new std::thread(Build::TryBuildProject, "Build/"); })
		}));

#ifdef ENGINE_CSHARP
	RegisterNewButtonCategory(ButtonCategory("C#",
		{
			ButtonCategory::Button("Reload C#", Editor::CurrentUI->Textures[12], []() 
				{ new std::thread(EditorUI::RebuildAndHotReload); }),
			ButtonCategory::Button("Run", Editor::CurrentUI->Textures[21], []()
				{
					new std::thread([]() {
						EditorUI::LaunchInEditor();
					});
				})
		}));
#endif

	GenerateButtons();
}

void Toolbar::UpdateLayout()
{
}

void Toolbar::Tick()
{
	UpdatePanel();
}

void Toolbar::OnButtonClicked(int Index)
{
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