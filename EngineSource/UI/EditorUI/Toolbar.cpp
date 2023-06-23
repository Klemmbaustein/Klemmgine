#if EDITOR
#include "Toolbar.h"
#include <Engine/Log.h>
#include <UI/UIText.h>
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Scene.h>
#include <thread>
#include <UI/EditorUI/Viewport.h>
#include <Engine/Build/Build.h>

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
		TabBackground->AddChild((new UIBackground(true, 0, UIColors[2] * 0.5, Vector2(0.001, 0.2)))
			->SetPadding(0.01, 0.01, 0.02, 0.01));
		auto Elem = new UIBox(false, 0);
		Elem->Align = UIBox::E_REVERSE;
		Elem->SetPadding(0.0, 0.0, 0, 0);
		Elem->AddChild((new UIText(0.4, UIColors[2] * 0.7, Buttons[i].Name, Editor::CurrentUI->EngineUIText))->SetPadding(0));
		TabBackground->AddChild(Elem);

		UIBox* ButtonBackground = new UIBox(true, 0);
		Elem->AddChild(ButtonBackground);
		ButtonBackground->SetPadding(0);

		size_t j = 0;
		for (auto& btn : Buttons[i].Buttons)
		{
			if (!btn.IsVisible)
			{
				continue;
			}
			ButtonBackground->AddChild((new UIBackground(false, 0, UIColors[3], Vector2(0.1)))
				->SetBorder(UIBox::E_ROUNDED, 0.5)
				->SetSizeMode(UIBox::E_PIXEL_RELATIVE)
				->AddChild((new UIText(0.35, UIColors[2], btn.Name, Editor::CurrentUI->EngineUIText))
					->SetPadding(0.005))
				->AddChild((new UIButton(true, 0, 1, this, i * MAX_CATEGORY_BUTTONS + j))
					->SetUseTexture(true, btn.Texture)
					->SetMinSize(0.075)
					->SetPadding(0.015)
					->SetSizeMode(UIBox::E_PIXEL_RELATIVE)));
			j++;
		}
	}
	TabBackground->AddChild((new UIBackground(true, 0, UIColors[2] * 0.5, Vector2(0.001, 0.2)))
		->SetPadding(0.01, 0.01, 0.02, 0.01));
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

Toolbar::Toolbar(Vector3* Colors, Vector2 Position, Vector2 Scale) : EditorPanel(Colors, Position, Scale, Vector2(0.8, 0.22), Vector2(2, 0.45))
{
	ToolbarInstance = this;
	RegisterNewButtonCategory(ButtonCategory("Scene", 
		{
			ButtonCategory::Button("Save", Editor::CurrentUI->Textures[2], []() {
					EditorUI::SaveCurrentScene();
				}),
			ButtonCategory::Button("Wireframe", Editor::CurrentUI->Textures[1], []() {
					Log::Print("Toggled wireframe", Vector3(0.3, 0.4, 1));
					Graphics::IsWireframe = !Graphics::IsWireframe;
				})

		}));
	RegisterNewButtonCategory(ButtonCategory("Project",
		{
			
			ButtonCategory::Button("Settings", Editor::CurrentUI->Textures[20], []() { Viewport::ViewportInstance->OpenTab(6, "Settings.setting"); }),
			ButtonCategory::Button("Build", Editor::CurrentUI->Textures[3], []() { new std::thread(Build::TryBuildProject, "Build/"); })

		}));

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