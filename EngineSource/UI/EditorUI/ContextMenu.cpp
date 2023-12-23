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
	UpdateLayout();
}

UITextField* ContextMenu::GenerateTextField(std::string Content, int Index)
{
	auto NewElement = new UITextField(0, 0.2f, this, Index, Editor::CurrentUI->EngineUIText);
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

	auto SeperatorArrow = new UIBackground(true, Vector2(0), 0, 0.035f);
	SeperatorArrow->SetPadding(0, 0, 0.01f, 0);
	SeperatorArrow->SetUseTexture(true,
		Editor::CurrentUI->CollapsedItems.contains(Prefix + Name) ? Editor::CurrentUI->Textures[14] : Editor::CurrentUI->Textures[13])
		->SetSizeMode(UIBox::SizeMode::PixelRelative);
	SeperatorBorder->AddChild(SeperatorArrow);

	auto SeperatorText = new UIText(0.45f, 0, Name, Editor::CurrentUI->EngineUIText);
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
		GenerateSectionElement(i, ContextObject, Name);
	}
}

template<typename T>
static size_t VecSize(void* vec)
{
	return ((std::vector<T>*)vec)->size();
}

template<typename T>
std::vector<T>& GetVec(void* vec)
{
	return *(std::vector<T>*)vec;
}

void ContextMenu::GenerateSectionElement(ContextMenuSection Element, WorldObject* ContextObject, std::string Name)
{
	UIBox* NewElement = nullptr;
	UIText* NewElementText = new UIText(0.4f, UIColors[2], Element.Name, Editor::CurrentUI->EngineUIText);
	NewElementText->SetPadding(0.01f, 0.005f, 0.02f, 0.005f);
	BackgroundBox->AddChild(NewElementText);
	int ElemIndex = Name == "Object" ? -2 : -1;
	UIVectorField::VecType VectorType = UIVectorField::VecType::xyz;

	bool IsList = Element.Type & Type::List;

	size_t NumElements = 1;

	if (IsList)
	{
		switch (Element.Type & ~Type::List)
		{
		case Type::Vector3Color:
		case Type::Vector3:
		case Type::Vector3Rotation:
			NumElements = VecSize<Vector3>(Element.Variable);
			break;
		case Type::Int:
			NumElements = VecSize<int>(Element.Variable);
			break;
		case Type::Float:
			NumElements = VecSize<float>(Element.Variable);
			break;
		case Type::String:
			NumElements = VecSize<std::string>(Element.Variable);
			break;
		case Type::Bool:
			NumElements = VecSize<bool>(Element.Variable);
			break;
		default:
			break;
		}
	}

	for (size_t i = 0; i < NumElements; i++)
	{
		void* val = Element.Variable;

		if (IsList)
		{
			switch (Element.Type & ~Type::List)
			{
			case Type::Vector3Color:
			case Type::Vector3:
			case Type::Vector3Rotation:
				val = &GetVec<Vector3>(Element.Variable).at(i);
				break;
			case Type::Int:
				val = &GetVec<int>(Element.Variable).at(i);
				break;
			case Type::Float:
				val = &GetVec<float>(Element.Variable).at(i);
				break;
			case Type::String:
				val = &GetVec<std::string>(Element.Variable).at(i);
				break;
			default:
				break;
			}
		}

		switch (Element.Type & ~Type::List)
		{
			// Vector3_Colors and Vector3s both use VectorFields, so we treat them the same
		case Type::Vector3Color:
			VectorType = UIVectorField::VecType::rgb;
			[[fallthrough]];
		case Type::Vector3Rotation:
			if (VectorType == UIVectorField::VecType::xyz)
			{
				VectorType = UIVectorField::VecType::PitchYawRoll;
			}
			[[fallthrough]];
		case Type::Vector3:
			NewElement = new UIVectorField(0, *(Vector3*)val, this, ElemIndex, Editor::CurrentUI->EngineUIText);
			NewElement->SetPadding(0.005f, 0, 0.02f, 0);
			((UIVectorField*)NewElement)->SetValueType(VectorType);
			break;
		case Type::Float:
			NewElement = GenerateTextField(EditorUI::ToShortString(*((float*)val)), ElemIndex);
			break;
		case Type::Int:
			NewElement = GenerateTextField(std::to_string(*((int*)val)), ElemIndex);

			break;
		case Type::String:
			NewElement = GenerateTextField(*((std::string*)val), ElemIndex);
			break;
		case Type::Bool:
		{
			bool Value = false;
			if (IsList)
			{
				Value = GetVec<bool>(Element.Variable).at(i);
			}
			else
			{
				Value = *((bool*)val);
			}
			NewElement = new UIButton(true, 0, 0.75f, this, ElemIndex);
			NewElement->SetSizeMode(UIBox::SizeMode::PixelRelative);
			NewElement->SetMinSize(0.04f);
			NewElement->SetBorder(UIBox::BorderType::Rounded, 0.3f);
			NewElement->SetPadding(0.01f, 0.01f, 0.02f, 0.01f);
			if (Value)
			{
				((UIButton*)NewElement)->SetUseTexture(true, Editor::CurrentUI->Textures[16]);
			}
		}
			break;
		default:
			break;
		}
		if (NewElement)
		{
			BackgroundBox->AddChild(NewElement);
			ContextButtons.push_back(NewElement);
			ContextSettings.push_back(Element);
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
		size_t IteratedElement = 0;

		for (size_t i = 0; i < ContextButtons.size(); i++)
		{
			if (ContextSettings[IteratedElement].Type & Type::List)
			{
				size_t NumElements = 1;

				auto& Element = ContextSettings[IteratedElement];

				switch (Element.Type & ~Type::List)
				{
				case Type::Vector3Color:
				case Type::Vector3:
				case Type::Vector3Rotation:
					NumElements = VecSize<Vector3>(Element.Variable);
					break;
				case Type::Int:
					NumElements = VecSize<int>(Element.Variable);
					break;
				case Type::Float:
					NumElements = VecSize<float>(Element.Variable);
					break;
				case Type::String:
					NumElements = VecSize<std::string>(Element.Variable);
					break;
				case Type::Bool:
					NumElements = VecSize<bool>(Element.Variable);
					break;
				default:
					break;
				}

				for (size_t j = 0; j < NumElements; j++)
				{
					try
					{
						switch (Element.Type & ~Type::List)
						{
						case Type::Vector3Color:
						case Type::Vector3Rotation:
						case Type::Vector3:
							if (Element.Normalized)
								GetVec<Vector3>(Element.Variable).at(j) = ((UIVectorField*)ContextButtons[i])->GetValue().Normalize();
							else
								GetVec<Vector3>(Element.Variable).at(j) = ((UIVectorField*)ContextButtons[i])->GetValue();
							break;
						case Type::Float:
							GetVec<float>(Element.Variable).at(j) = std::stof(((UITextField*)ContextButtons[i])->GetText());
							break;
						case Type::Int:
							GetVec<int>(Element.Variable).at(j) = std::stoi(((UITextField*)ContextButtons[i])->GetText());
							break;
						case Type::String:
							GetVec<std::string>(Element.Variable).at(j) = ((UITextField*)ContextButtons[i])->GetText();

							if (((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects.size()
								&& ContextSettings[IteratedElement].Variable == &((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects[0]->Name)
							{
								Editor::CurrentUI->UIElements[5]->UpdateLayout();
							}
							break;
						case Type::Bool:
							if (((UIButton*)ContextButtons[i])->GetIsHovered())
							{
								bool val = GetVec<bool>(Element.Variable).at(j);
								GetVec<bool>(Element.Variable).at(j) = !val;
							}
							break;
						default:
							break;
						}
					}
					catch (std::exception)
					{
					}
					i++;
				}
			}
			else
			{
				try
				{
					switch (ContextSettings[IteratedElement].Type & ~Type::List)
					{
					case Type::Vector3Color:
					case Type::Vector3Rotation:
					case Type::Vector3:
						if (ContextSettings[IteratedElement].Normalized) 
							*(Vector3*)(ContextSettings[IteratedElement].Variable) = ((UIVectorField*)ContextButtons[i])->GetValue().Normalize();
						else
							*(Vector3*)(ContextSettings[IteratedElement].Variable) = ((UIVectorField*)ContextButtons[i])->GetValue();
						break;
					case Type::Float:
						*(float*)(ContextSettings[IteratedElement].Variable) = std::stof(((UITextField*)ContextButtons[i])->GetText());
						break;
					case Type::Int:
						*(int*)(ContextSettings[IteratedElement].Variable) = std::stoi(((UITextField*)ContextButtons[i])->GetText());
						break;
					case Type::String:
						*(std::string*)(ContextSettings[IteratedElement].Variable) = ((UITextField*)ContextButtons[i])->GetText();
						if (((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects.size()
							&& ContextSettings[IteratedElement].Variable == &((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects[0]->Name)
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
			IteratedElement++;
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
		WorldObject* SelectedObject = ((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects.at(0);
		BackgroundBox->AddChild((new UIText(0.5f, UIColors[2], "Object: " + SelectedObject->GetName(), Editor::CurrentUI->EngineUIText))
			->SetPadding(0.01f));
		GenerateSection(
			{
				ContextMenuSection(&SelectedObject->GetTransform().Location, Type::Vector3, "Location"),
				ContextMenuSection(&SelectedObject->GetTransform().Rotation, Type::Vector3Rotation, "Rotation"),
				ContextMenuSection(&SelectedObject->GetTransform().Scale, Type::Vector3, "Scale"),
				ContextMenuSection(&SelectedObject->Name, Type::String, "Name"),
			},
			"Object", ((Viewport*)Editor::CurrentUI->UIElements[4])->SelectedObjects.at(0), 0);

		std::map<std::string, std::vector<ContextMenuSection>> Categories;

		for (WorldObject::Property i : SelectedObject->Properties)
		{
			if (i.PType != WorldObject::Property::PropertyType::EditorProperty)
			{
				continue;
			}
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
		BackgroundBox->AddChild((new UIText(0.5f, UIColors[2], "Scene: "
			+ FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene), Editor::CurrentUI->EngineUIText))
			->SetPadding(0.01f));
		GenerateSection(
			{
				ContextMenuSection(&Graphics::WorldSun.Rotation, Type::Vector3Rotation, "Rotation"),
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