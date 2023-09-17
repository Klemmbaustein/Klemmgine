#if EDITOR
#include "ContextMenu.h"
#include <UI/EditorUI/UIVectorField.h>
#include <UI/UIButton.h>
#include <UI/UITextField.h>
#include <UI/UIText.h>
#include <UI/UIScrollBox.h>
#include <UI/EditorUI/EditorUI.h>
#include <UI/EditorUI/Viewport.h>
#include <Engine/Utility/FileUtility.h>
#include <Engine/Scene.h>
#include <Engine/Log.h>
#include <Rendering/Utility/BakedLighting.h>

ContextMenu::ContextMenu(Vector3* Colors, Vector2 Position, Vector2 Scale) : EditorPanel(Colors, Position, Scale, Vector2(0.3f, 0.5f))
{
	BackgroundBox = new UIScrollBox(false, 0, true);
	BackgroundBox->SetPadding(0.01f);
	BackgroundBox->SetAlign(UIBox::Align::Reverse);
	UpdateLayout();
}

UITextField* ContextMenu::GenerateTextField(std::string Content, int Index)
{
	auto NewElement = new UITextField(true, 0, 0.2f, this, Index, Editor::CurrentUI->EngineUIText);
	((UITextField*)NewElement)->SetText(Content);
	((UITextField*)NewElement)->SetTextSize(0.4f);
	NewElement->SetPadding(0.005f, 0.005f, 0.02f, 0.005f);
	NewElement->SetBorder(UIBox::BorderType::Rounded, 0.5f);
	NewElement->SetMinSize(Vector2(0.265f, 0.04f));
	NewElement->SetMaxSize(Vector2(0.3f, 0.04f));
	return NewElement;
}

void ContextMenu::GenerateSection(std::vector<ContextMenuSection> Section, std::string Name, WorldObject* ContextObject, unsigned int Index)
{
	auto SeperatorBorder = new UIButton(true, 0, 0.5f, this, Index);

	std::string Prefix = ContextObject ? "OBJ_CAT_" : "SCN_";

	auto SeperatorArrow = new UIBackground(true, Vector2(0), 0, Vector2(1, Graphics::AspectRatio) / 45);
	SeperatorArrow->SetPadding(0, 0, 0.01f, 0);
	SeperatorArrow->SetUseTexture(true,
		Editor::CurrentUI->CollapsedItems.contains(Prefix + Name) ? Editor::CurrentUI->Textures[14] : Editor::CurrentUI->Textures[13]);
	//SeperatorArrow->SetTryFill(true);
	SeperatorBorder->AddChild(SeperatorArrow);

	auto SeperatorText = new UIText(0.5f, 0, Name, Editor::CurrentUI->EngineUIText);
	SeperatorText->SetTryFill(true);
	SeperatorText->SetPadding(0.005f);
	SeperatorBorder->SetPadding(0.015f, 0.015f, 0, 0);
	SeperatorBorder->SetMinSize(Vector2(Scale.X, 0));
	BackgroundBox->AddChild(SeperatorBorder);
	SeperatorBorder->AddChild(SeperatorText);

	ContextCategories.push_back(Name);
	if (Editor::CurrentUI->CollapsedItems.contains(Prefix + Name))
	{
		return;
	}

	for (const auto& i : Section)
	{
		UIBox* NewElement = nullptr;
		UIText* NewElementText = new UIText(0.45f, UIColors[2], i.Name, Editor::CurrentUI->EngineUIText);
		NewElementText->SetPadding(0.005f, 0.005f, 0.02f, 0.005f);
		BackgroundBox->AddChild(NewElementText);
		int ElemIndex = Name == "Object" ? -2 : -1;
		UIVectorField::VecType VectorType = UIVectorField::VecType::xyz;
		switch (i.Type)
		{
			// Vector3_Colors and Vector3s both use VectorFields, so we basically treat them the same
		case Type::Vector3Color:
			VectorType = UIVectorField::VecType::rgb;
		case Type::Vector3Rotation:
			if (VectorType == UIVectorField::VecType::xyz)
			{
				VectorType = UIVectorField::VecType::PitchYawRoll;
			}
		case Type::Vector3:
			NewElement = new UIVectorField(0, *(Vector3*)i.Variable, this, ElemIndex, Editor::CurrentUI->EngineUIText);
			NewElement->SetPadding(0.005f, 0, 0.02f, 0);
			((UIVectorField*)NewElement)->SetValueType(VectorType);
			break;
		case Type::Float:
			NewElement = GenerateTextField(EditorUI::ToShortString(*((float*)i.Variable)), ElemIndex);
			break;
		case Type::Int:
			NewElement = GenerateTextField(std::to_string(*((int*)i.Variable)), ElemIndex);

			break;
		case Type::String:
			NewElement = GenerateTextField(*((std::string*)i.Variable), ElemIndex);
			break;
		case Type::Bool:
			NewElement = new UIButton(true, 0, 0.75f, this, ElemIndex);
			NewElement->SetSizeMode(UIBox::SizeMode::PixelRelative);
			NewElement->SetMinSize(0.04f);
			NewElement->SetBorder(UIBox::BorderType::Rounded, 0.3f);
			NewElement->SetPadding(0.01f, 0.01f, 0.02f, 0.01f);
			if (*((bool*)i.Variable))
			{
				((UIButton*)NewElement)->SetUseTexture(true, Editor::CurrentUI->Textures[16]);
			}
			break;
		default:
			break;
		}
		if (NewElement)
		{
			BackgroundBox->AddChild(NewElement);
			ContextButtons.push_back(NewElement);
			ContextSettings.push_back(i);
		}
	}
}

void ContextMenu::Tick()
{
	UpdatePanel();	
	BackgroundBox->SetPosition(TabBackground->GetPosition());

}

void ContextMenu::OnButtonClicked(int Index)
{
	if (Index >= 0)
	{
		std::string Name = ContextCategories.at(Index);
		Name = (Viewport::ViewportInstance->SelectedObjects.size() ? "OBJ_CAT_" : "SCN_") + Name;
		if (Editor::CurrentUI->CollapsedItems.contains(Name))
		{
			Editor::CurrentUI->CollapsedItems.erase(Name);
		}
		else
		{
			Editor::CurrentUI->CollapsedItems.insert(Name);
		}
		UpdateLayout();
	}
	if (Index == -1 || Index == -2)
	{
		for (size_t i = 0; i < ContextButtons.size(); i++)
		{
			try
			{
				switch (ContextSettings[i].Type)
				{
				case Type::Vector3Color:
				case Type::Vector3Rotation:
				case Type::Vector3:
					if (ContextSettings[i].Normalized) *(Vector3*)(ContextSettings[i].Variable) = ((UIVectorField*)ContextButtons[i])->GetValue().Normalize();
					else
						*(Vector3*)(ContextSettings[i].Variable) = ((UIVectorField*)ContextButtons[i])->GetValue();
					break;
				case Type::Float:
					*(float*)(ContextSettings[i].Variable) = std::stof(((UITextField*)ContextButtons[i])->GetText());
					break;
				case Type::Int:
					*(int*)(ContextSettings[i].Variable) = std::stoi(((UITextField*)ContextButtons[i])->GetText());
					break;
				case Type::String:
					*(std::string*)(ContextSettings[i].Variable) = ((UITextField*)ContextButtons[i])->GetText();
					if (((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects.size()
						&& ContextSettings[i].Variable == &((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects[0]->Name)
					{
						Editor::CurrentUI->UIElements[5]->UpdateLayout();
					}
					break;
				case Type::Bool:
					if (((UIButton*)ContextButtons[i])->GetIsHovered())
					{
						*(bool*)ContextSettings[i].Variable = !(*(bool*)ContextSettings[i].Variable);
					}
					break;
				default:
					break;
				}
			}
			catch (std::exception)
			{
			}
		}
		if (((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects.size()
			&& ((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects[0]->Properties.size()
			&& Index == -1)
		{
			((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects[0]->OnPropertySet();
		}
		ChangedScene = true;
		UpdateLayout();
	}
}

void ContextMenu::UpdateLayout()
{
	BackgroundBox->SetPosition(TabBackground->GetPosition());
	BackgroundBox->SetMinSize(Scale);
	BackgroundBox->SetMaxSize(Scale);
	BackgroundBox->DeleteChildren();
	ContextSettings.clear();
	ContextButtons.clear();
	ContextCategories.clear();


	if (Editor::CurrentUI->UIElements[4] && ((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects.size() > 0)
	{
		Properties.clear();
		WorldObject* SelectedObject = ((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects[0];
		BackgroundBox->AddChild((new UIText(0.6f, UIColors[2], "Object: " + SelectedObject->GetName(), Editor::CurrentUI->EngineUIText))
			->SetPadding(0.01f));
		GenerateSection(
			{
				ContextMenuSection(&SelectedObject->GetTransform().Location, Type::Vector3, "Location"),
				ContextMenuSection(&SelectedObject->GetTransform().Rotation, Type::Vector3Rotation, "Rotation"),
				ContextMenuSection(&SelectedObject->GetTransform().Scale, Type::Vector3, "Scale"),
				ContextMenuSection(&SelectedObject->Name, Type::String, "Name"),
			},
			"Object", ((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects[0], 0);

		std::map<std::string, std::vector<ContextMenuSection>> Categories;

		for (Objects::Property i : SelectedObject->Properties)
		{
			auto Colon = i.Name.find_last_of(":");
			std::string CategoryName = SelectedObject->GetObjectDescription().Name;
			if (Colon != std::string::npos)
			{
				CategoryName = i.Name.substr(0, Colon);
				i.Name = i.Name.substr(Colon + 1);
			}
			if (!Categories.contains(CategoryName))
			{
				Categories.insert(std::pair(CategoryName, std::vector<ContextMenuSection>({ ContextMenuSection(i.Data, i.Type, i.Name) })));
			}
			else
			{
				Categories[CategoryName].push_back(ContextMenuSection(i.Data, i.Type, i.Name));
			}
		}
		char Iterator = 1;
		for (auto& i : Categories)
		{
			GenerateSection(i.second, i.first, ((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects[0], Iterator);
			Iterator++;
		}
	}
	else
	{
		BackgroundBox->AddChild((new UIText(0.6f, UIColors[2], "Scene: " + FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene), Editor::CurrentUI->EngineUIText))
			->SetPadding(0.01f));
		GenerateSection(
			{
				ContextMenuSection(&Graphics::WorldSun.Direction, Type::Vector3Rotation, "Direction", true),
				ContextMenuSection(&Graphics::WorldSun.SunColor, Type::Vector3Color, "Color"),
				ContextMenuSection(&Graphics::WorldSun.Intensity, Type::Float, "Intensity")
			},
			"Sun", nullptr, 0);
		GenerateSection(
			{
				ContextMenuSection(&Graphics::WorldSun.AmbientColor, Type::Vector3Color, "Color"),
				ContextMenuSection(&Graphics::WorldSun.AmbientIntensity, Type::Float, "Intensity")
			},
			"Ambient light", nullptr, 1);
		GenerateSection(
			{
				ContextMenuSection(&Graphics::WorldFog.FogColor, Type::Vector3Color, "Color"),
				ContextMenuSection(&Graphics::WorldFog.Distance, Type::Float, "Start distance"),
				ContextMenuSection(&Graphics::WorldFog.Falloff, Type::Float, "Falloff"),
				ContextMenuSection(&Graphics::WorldFog.MaxDensity, Type::Float, "Max density")
			},
			"Fog", nullptr, 2);
		GenerateSection(
			{
				ContextMenuSection(&Graphics::MainFramebuffer->ReflectionCubemapName, Type::String, "Cubemap file"),
			},
			"Reflection", nullptr, 3);
	}

}

#endif