#if EDITOR
#include "ContextMenu.h"
#include <UI/EditorUI/UIVectorField.h>
#include <UI/UIButton.h>
#include <UI/UITextField.h>
#include <UI/UIText.h>
#include <UI/UIScrollBox.h>
#include <UI/EditorUI/EditorUI.h>
#include <UI/EditorUI/Viewport.h>
#include <Engine/FileUtility.h>
#include <Engine/Scene.h>
#include <Engine/Log.h>

ContextMenu::ContextMenu(Vector3* Colors, Vector2 Position, Vector2 Scale) : EditorPanel(Colors, Position, Scale, Vector2(0.3, 0.5))
{
	BackgroundBox = new UIScrollBox(false, 0, 25);
	BackgroundBox->SetPadding(0.01);
	BackgroundBox->Align = UIBox::E_REVERSE;
	UpdateLayout();
}

UITextField* ContextMenu::GenerateTextField(std::string Content)
{
	auto NewElement = new UITextField(true, 0, 0.2, this, -1, Editor::CurrentUI->EngineUIText);
	((UITextField*)NewElement)->SetText(Content);
	((UITextField*)NewElement)->SetTextSize(0.4);
	NewElement->SetPadding(0.005, 0.005, 0.02, 0.005);
	NewElement->SetBorder(UIBox::E_ROUNDED, 0.5);
	NewElement->SetMinSize(Vector2(0.265, 0.04));
	NewElement->SetMaxSize(Vector2(0.3, 0.04));
	return NewElement;
}

void ContextMenu::GenerateSection(std::vector<ContextMenuSection> Section, std::string Name, WorldObject* ContextObject, unsigned int Index)
{
	auto SeperatorBorder = new UIButton(true, 0, 0.5, this, Index);

	std::string Prefix = ContextObject ? "OBJ_CAT_" : "SCN_";

	auto SeperatorArrow = new UIBackground(true, Vector2(0), 0, Vector2(1, Graphics::AspectRatio) / 45);
	SeperatorArrow->SetPadding(0, 0, 0.01, 0);
	SeperatorArrow->SetUseTexture(true,
		Editor::CurrentUI->CollapsedItems.contains(Prefix + Name) ? Editor::CurrentUI->Textures[14] : Editor::CurrentUI->Textures[13]);
	//SeperatorArrow->SetTryFill(true);
	SeperatorBorder->AddChild(SeperatorArrow);

	auto SeperatorText = new UIText(0.5, 0, Name, Editor::CurrentUI->EngineUIText);
	SeperatorText->SetTryFill(true);
	SeperatorText->SetPadding(0.005);
	SeperatorBorder->SetPadding(0.03, 0.03, 0, 0);
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
		UIText* NewElementText = new UIText(0.45, UIColors[2], i.Name, Editor::CurrentUI->EngineUIText);
		NewElementText->SetPadding(0.005, 0.005, 0.02, 0.005);
		BackgroundBox->AddChild(NewElementText);
		switch (i.Type)
		{
			// Vector3_Colors and Vector3s both use VectorFields, so we basically treat them the same
		case Type::E_VECTOR3_COLOR:
		case Type::E_VECTOR3:
			NewElement = new UIVectorField(0, *(Vector3*)i.Variable, this, -1, Editor::CurrentUI->EngineUIText);
			NewElement->SetPadding(0.005, 0, 0.02, 0);
			// Here we tell the VectorField to use RGB values instead of XYZ if required
			((UIVectorField*)NewElement)->SetValueType(i.Type == Type::E_VECTOR3 ? UIVectorField::E_XYZ : UIVectorField::E_RGB);
			break;
		case Type::E_FLOAT:
			NewElement = GenerateTextField(EditorUI::ToShortString(*((float*)i.Variable)));
			break;
		case Type::E_INT:
			NewElement = GenerateTextField(std::to_string(*((int*)i.Variable)));

			break;
		case Type::E_STRING:
			NewElement = GenerateTextField(*((std::string*)i.Variable));
			break;
		case Type::E_BOOL:
			NewElement = new UIButton(true, 0, 0.75, this, -1);
			NewElement->SetSizeMode(UIBox::E_PIXEL_RELATIVE);
			NewElement->SetMinSize(0.04);
			NewElement->SetBorder(UIBox::E_ROUNDED, 0.3);
			NewElement->SetPadding(0.01, 0.01, 0.02, 0.01);
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
	if (Index == -1)
	{
		for (size_t i = 0; i < ContextButtons.size(); i++)
		{
			try
			{
				switch (ContextSettings[i].Type)
				{
				case Type::E_VECTOR3_COLOR:
				case Type::E_VECTOR3:
					if (ContextSettings[i].Normalized) *(Vector3*)(ContextSettings[i].Variable) = ((UIVectorField*)ContextButtons[i])->GetValue().Normalize();
					else
						*(Vector3*)(ContextSettings[i].Variable) = ((UIVectorField*)ContextButtons[i])->GetValue();
					break;
				case Type::E_FLOAT:
					*(float*)(ContextSettings[i].Variable) = std::stof(((UITextField*)ContextButtons[i])->GetText());
					break;
				case Type::E_INT:
					*(int*)(ContextSettings[i].Variable) = std::stof(((UITextField*)ContextButtons[i])->GetText());
					break;
				case Type::E_STRING:
					*(std::string*)(ContextSettings[i].Variable) = ((UITextField*)ContextButtons[i])->GetText();
					if (((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects.size()
						&& ContextSettings[i].Variable == &((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects[0]->Name)
					{
						Editor::CurrentUI->UIElements[5]->UpdateLayout();
					}
					break;
				case Type::E_BOOL:
					if (((UIButton*)ContextButtons[i])->GetIsHovered())
					{
						*(bool*)ContextSettings[i].Variable = !(*(bool*)ContextSettings[i].Variable);
					}
					break;
				default:
					break;
				}
			}
			catch (std::exception& e)
			{

			}
		}
		if (((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects.size()
			&& ((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects[0]->Properties.size())
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
		BackgroundBox->AddChild((new UIText(0.6, UIColors[2], "Object: " + SelectedObject->GetName(), Editor::CurrentUI->EngineUIText))->SetPadding(0.01));
		GenerateSection(
			{
				ContextMenuSection(&SelectedObject->GetTransform().Location, Type::E_VECTOR3, "Location"),
				ContextMenuSection(&SelectedObject->GetTransform().Rotation, Type::E_VECTOR3, "Rotation"),
				ContextMenuSection(&SelectedObject->GetTransform().Scale, Type::E_VECTOR3, "Scale"),
				ContextMenuSection(&SelectedObject->Name, Type::E_STRING, "Name"),
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
		BackgroundBox->AddChild((new UIText(0.6, UIColors[2], "Scene: " + FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene), Editor::CurrentUI->EngineUIText))
			->SetPadding(0.01));
		GenerateSection(
			{
				ContextMenuSection(&Graphics::WorldSun.Direction, Type::E_VECTOR3, "Direction", true),
				ContextMenuSection(&Graphics::WorldSun.SunColor, Type::E_VECTOR3_COLOR, "Color"),
				ContextMenuSection(&Graphics::WorldSun.Intensity, Type::E_FLOAT, "Intensity")
			},
			"Sun", nullptr, 0);
		GenerateSection(
			{
				ContextMenuSection(&Graphics::WorldSun.AmbientColor, Type::E_VECTOR3_COLOR, "Color"),
				ContextMenuSection(&Graphics::WorldSun.AmbientIntensity, Type::E_FLOAT, "Intensity")
			},
			"Ambient light", nullptr, 1);
		GenerateSection(
			{
				ContextMenuSection(&Graphics::WorldFog.FogColor, Type::E_VECTOR3_COLOR, "Color"),
				ContextMenuSection(&Graphics::WorldFog.Distance, Type::E_FLOAT, "Start distance"),
				ContextMenuSection(&Graphics::WorldFog.Falloff, Type::E_FLOAT, "Falloff"),
				ContextMenuSection(&Graphics::WorldFog.MaxDensity, Type::E_FLOAT, "Max density")
			},
			"Fog", nullptr, 2);
		GenerateSection(
			{
				ContextMenuSection(&Graphics::MainFramebuffer->ReflectionCubemapName, Type::E_STRING, "Cubemap file"),
			},
			"Reflection", nullptr, 3);
	}

}

#endif