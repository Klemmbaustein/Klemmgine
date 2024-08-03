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
#include <Engine/Subsystem/Scene.h>
#include <Engine/Log.h>
#include <Rendering/RenderSubsystem/BakedLighting.h>
#include <Engine/Utility/StringUtility.h>
#include <Engine/Application.h>
#include "ObjectList.h"
#include <Objects/CSharpObject.h>

ContextMenu::ContextMenu(EditorPanel* Parent, bool IsScene) : EditorPanel(Parent, IsScene ? "Scene" : "Object Properties", IsScene ? "context_menu_scene" : "context_menu_obj")
{
	IsObject = !IsScene;
	BackgroundBox = new UIScrollBox(UIBox::Orientation::Vertical, 0, true);
	PanelMainBackground->AddChild(BackgroundBox);
}

UITextField* ContextMenu::GenerateTextField(std::string Content, int Index)
{
	auto NewElement = new UITextField(0, EditorUI::UIColors[1], this, Index, EditorUI::Text);
	((UITextField*)NewElement)->SetText(Content);
	((UITextField*)NewElement)->SetTextSize(0.4f);
	NewElement->SetTextColor(EditorUI::UIColors[2]);
	NewElement->SetPadding(0.005f, 0.005f, 0.02f, 0.005f);
	NewElement->SetMinSize(Vector2(Scale.X - 0.04f, 0.04f));
	NewElement->SetMaxSize(Vector2(Scale.X - 0.04f, 0.04f));
	return NewElement;
}

void ContextMenu::GenerateSection(std::vector<ContextMenuSection> Section, std::string Name, SceneObject* ContextObject, unsigned int Index)
{
	auto SeparatorBorder = new UIButton(UIBox::Orientation::Horizontal, 0, 0.5f, this, Index);

	StrUtil::ReplaceChar(Name, '\n', "");

	std::string Prefix = ContextObject ? "OBJ_CAT_" : "SCN_";

	bool Collapsed = CollapsedItems.contains(Prefix + Name);

	auto SeparatorArrow = new UIBackground(UIBox::Orientation::Horizontal, Vector2(0), EditorUI::UIColors[2], 0.035f);
	SeparatorArrow->SetPadding(0, 0, 0.005f, 0);
	SeparatorArrow->SetUseTexture(true,
			Collapsed ? Application::EditorInstance->Textures[14] : Application::EditorInstance->Textures[13])
		->SetSizeMode(UIBox::SizeMode::AspectRelative);

	SeparatorBorder
		->SetVerticalAlign(UIBox::Align::Centered)
		->AddChild(SeparatorArrow);

	auto SeparatorText = new UIText(0.45f, EditorUI::UIColors[2], Name, EditorUI::Text);
	SeparatorText->SetPadding(0.005f);
	SeparatorBorder->SetPadding(0.015f, 0, 0, 0);
	SeparatorBorder->SetMinSize(Vector2(Scale.X, 0));
	SeparatorBorder->SetOpacity(0);
	BackgroundBox->AddChild(SeparatorBorder);
	SeparatorBorder->AddChild(SeparatorText);
	BackgroundBox->AddChild((new UIBackground(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2], Vector2(Scale.X - 0.005f, 2.0f / Graphics::WindowResolution.Y)))
		->SetPadding(0, 0.005f, 0.0025f, 0));
	ContextCategories.push_back(Name);
	if (Collapsed)
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

void ContextMenu::GenerateCSharpProperty(const ContextMenu::ContextMenuSection& Element, SceneObject* ContextObject)
{
#ifdef ENGINE_CSHARP
	UIBox* NewElement = nullptr;
	CSharpObject* obj = static_cast<CSharpObject*>(ContextObject);
	std::string Value = obj->GetProperty(Element.Name);
	UIVectorField::VecType VectorType = UIVectorField::VecType::xyz;

	if (Value.find("\r") == std::string::npos)
	{
		switch (Element.NativeType)
		{
		case NativeType::Vector3Color:
			VectorType = UIVectorField::VecType::rgb;
			[[fallthrough]];
		case NativeType::Vector3Rotation:
			if (VectorType == UIVectorField::VecType::xyz)
			{
				VectorType = UIVectorField::VecType::PitchYawRoll;
			}
			[[fallthrough]];
		case NativeType::Vector3:
			NewElement = new UIVectorField(Scale.X - 0.04f, Vector3::FromString(Value), this, -1, EditorUI::Text);
			NewElement->SetPadding(0.005f, 0, 0.02f, 0);
			((UIVectorField*)NewElement)->SetValueType(VectorType);
			break;
		case NativeType::String:
		case NativeType::Float:
		case NativeType::Int:
			NewElement = GenerateTextField(Value, -1);
			break;
		case NativeType::Bool:
			NewElement = new UIButton(UIBox::Orientation::Horizontal, 0, 0.75f, this, -1);
			NewElement->SetSizeMode(UIBox::SizeMode::AspectRelative);
			NewElement->SetMinSize(0.04f);
			NewElement->SetBorder(UIBox::BorderType::Rounded, 0.3f);
			NewElement->SetPadding(0.01f, 0.01f, 0.02f, 0.01f);
			if (Value == "True")
			{
				((UIButton*)NewElement)->SetUseTexture(true, Application::EditorInstance->Textures[16]);
			}

			break;
		default:
			break;
		}
		if (NewElement)
		{
			BackgroundBox->AddChild(NewElement);
			ContextButtons.push_back(NewElement);
		}
		return;
	}
	return;
	auto Elements = StrUtil::SeparateString(Value, '\r');
	for (auto& i : Elements)
	{
		switch (Element.NativeType & ~NativeType::List)
		{
		case NativeType::Vector3Color:
			VectorType = UIVectorField::VecType::rgb;
			[[fallthrough]];
		case NativeType::Vector3Rotation:
			if (VectorType == UIVectorField::VecType::xyz)
			{
				VectorType = UIVectorField::VecType::PitchYawRoll;
			}
			[[fallthrough]];
		case NativeType::Vector3:
			NewElement = new UIVectorField(Scale.X - 0.04f, Vector3::FromString(i), this, -1, EditorUI::Text);
			NewElement->SetPadding(0.005f, 0, 0.02f, 0);
			((UIVectorField*)NewElement)->SetValueType(VectorType);
			break;
		case NativeType::String:
		case NativeType::Float:
		case NativeType::Int:
			NewElement = GenerateTextField(i, -1);
			break;
		default:
			break;
		}
		if (NewElement)
		{
			BackgroundBox->AddChild(NewElement);
			ContextButtons.push_back(NewElement);
		}
	}
#endif
}

void ContextMenu::GenerateSectionElement(ContextMenuSection Element, SceneObject* ContextObject, std::string Name)
{
	UIBox* NewElement = nullptr;
	UIText* NewElementText = new UIText(0.4f, EditorUI::UIColors[2], Element.Name, EditorUI::Text);
	NewElementText->SetPadding(0.01f, 0.005f, 0.02f, 0.005f);
	BackgroundBox->AddChild(NewElementText);
	int ElemIndex = Name == "Object" ? -2 : -1;
	UIVectorField::VecType VectorType = UIVectorField::VecType::xyz;

	bool IsList = Element.NativeType & NativeType::List;

	size_t NumElements = 1;

	if (IsList && Element.Variable)
	{
		switch (Element.NativeType & ~NativeType::List)
		{
		case NativeType::Vector3Color:
		case NativeType::Vector3:
		case NativeType::Vector3Rotation:
			NumElements = VecSize<Vector3>(Element.Variable);
			break;
		case NativeType::Int:
			NumElements = VecSize<int>(Element.Variable);
			break;
		case NativeType::Float:
			NumElements = VecSize<float>(Element.Variable);
			break;
		case NativeType::String:
			NumElements = VecSize<std::string>(Element.Variable);
			break;
		case NativeType::Bool:
			NumElements = VecSize<bool>(Element.Variable);
			break;
		default:
			break;
		}
	}

	for (size_t i = 0; i < NumElements; i++)
	{
		void* val = Element.Variable;

#ifdef ENGINE_CSHARP
		if (val == nullptr)
		{
			GenerateCSharpProperty(Element, ContextObject);
			continue;
		}
#endif
		if (IsList)
		{
			switch (Element.NativeType & ~NativeType::List)
			{
			case NativeType::Vector3Color:
			case NativeType::Vector3:
			case NativeType::Vector3Rotation:
				val = &GetVec<Vector3>(Element.Variable).at(i);
				break;
			case NativeType::Int:
				val = &GetVec<int>(Element.Variable).at(i);
				break;
			case NativeType::Float:
				val = &GetVec<float>(Element.Variable).at(i);
				break;
			case NativeType::String:
				val = &GetVec<std::string>(Element.Variable).at(i);
				break;
			default:
				break;
			}
		}

		switch (Element.NativeType & ~NativeType::List)
		{
			// Vector3_Colors and Vector3s both use VectorFields, so we treat them the same
		case NativeType::Vector3Color:
			VectorType = UIVectorField::VecType::rgb;
			[[fallthrough]];
		case NativeType::Vector3Rotation:
			if (VectorType == UIVectorField::VecType::xyz)
			{
				VectorType = UIVectorField::VecType::PitchYawRoll;
			}
			[[fallthrough]];
		case NativeType::Vector3:
			NewElement = new UIVectorField(Scale.X - 0.04f, *(Vector3*)val, this, ElemIndex, EditorUI::Text);
			NewElement->SetPadding(0.005f, 0, 0.02f, 0);
			((UIVectorField*)NewElement)->SetValueType(VectorType);
			break;
		case NativeType::Float:
			NewElement = GenerateTextField(EditorUI::ToShortString(*((float*)val)), ElemIndex);
			break;
		case NativeType::Int:
			NewElement = GenerateTextField(std::to_string(*((int*)val)), ElemIndex);

			break;
		case NativeType::String:
			NewElement = GenerateTextField(*((std::string*)val), ElemIndex);
			break;
		case NativeType::Bool:
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
			NewElement = new UIButton(UIBox::Orientation::Horizontal, 0, 0.75f, this, ElemIndex);
			NewElement->SetSizeMode(UIBox::SizeMode::AspectRelative);
			NewElement->SetMinSize(0.04f);
			NewElement->SetBorder(UIBox::BorderType::Rounded, 0.3f);
			NewElement->SetPadding(0.01f, 0.01f, 0.02f, 0.01f);
			if (Value)
			{
				((UIButton*)NewElement)->SetUseTexture(true, Application::EditorInstance->Textures[16]);
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
		}
	}
	ContextSettings.push_back(Element);
}

void ContextMenu::Tick()
{
	TickPanel();
	BackgroundBox->IsVisible = PanelMainBackground->IsVisible;
}

void ContextMenu::OnButtonClicked(int Index)
{
	if (Index >= 0)
	{
		std::string Name = ContextCategories.at(Index);
		Name = (IsObject ? "OBJ_CAT_" : "SCN_") + Name;
		if (CollapsedItems.contains(Name))
		{
			CollapsedItems.erase(Name);
		}
		else
		{
			CollapsedItems.insert(Name);
		}
		OnResized();
	}
	if (Index == -1 || Index == -2)
	{
		size_t IteratedElement = 0;

		for (size_t i = 0; i < ContextButtons.size(); i++)
		{
			if (!ContextSettings[IteratedElement].Variable)
			{
#ifdef ENGINE_CSHARP
				auto& Element = ContextSettings[IteratedElement];
				CSharpObject* obj = static_cast<CSharpObject*>(EditorUI::SelectedObjects[0]);

				if (ContextSettings[IteratedElement].NativeType & NativeType::List)
				{
					auto arr = StrUtil::SeparateString(obj->GetProperty(Element.Name), '\r');
					std::string Value;
					for (auto& arrElem : arr)
					{
						ENGINE_ASSERT(arrElem.find("\r") == std::string::npos, "");
						Value.append(((UITextField*)ContextButtons[i])->GetText());
						i++;
					}
					Value.pop_back();
					i--;
					obj->SetProperty(Element.Name, Value);
				}
				else
				{
					switch (Element.NativeType)
					{
					case NativeType::Int:
					case NativeType::Float:
					case NativeType::String:
						obj->SetProperty(Element.Name, ((UITextField*)ContextButtons[i])->GetText());
						break;
					case NativeType::Vector3:
					case NativeType::Vector3Color:
					case NativeType::Vector3Rotation:
						obj->SetProperty(Element.Name, ((UIVectorField*)ContextButtons[i])->GetValue().ToString());
						break;
					case NativeType::Bool:
						if (((UIButton*)ContextButtons[i])->GetIsHovered())
						{
							obj->SetProperty(Element.Name, obj->GetProperty(Element.Name) == "True" ? "False" : "True");
						}
						break;
					default:
						break;
					}
				}

				IteratedElement++;
				continue;
#endif
			}
			if (ContextSettings[IteratedElement].NativeType & NativeType::List)
			{
				size_t NumElements = 1;

				auto& Element = ContextSettings[IteratedElement];

				switch (Element.NativeType & ~NativeType::List)
				{
				case NativeType::Vector3Color:
				case NativeType::Vector3:
				case NativeType::Vector3Rotation:
					NumElements = VecSize<Vector3>(Element.Variable);
					break;
				case NativeType::Int:
					NumElements = VecSize<int>(Element.Variable);
					break;
				case NativeType::Float:
					NumElements = VecSize<float>(Element.Variable);
					break;
				case NativeType::String:
					NumElements = VecSize<std::string>(Element.Variable);
					break;
				case NativeType::Bool:
					NumElements = VecSize<bool>(Element.Variable);
					break;
				default:
					break;
				}

				for (size_t j = 0; j < NumElements; j++)
				{
					try
					{
						switch (Element.NativeType & ~NativeType::List)
						{
						case NativeType::Vector3Color:
						case NativeType::Vector3Rotation:
						case NativeType::Vector3:
							if (Element.Normalized)
								GetVec<Vector3>(Element.Variable).at(j) = ((UIVectorField*)ContextButtons[i])->GetValue().Normalize();
							else
								GetVec<Vector3>(Element.Variable).at(j) = ((UIVectorField*)ContextButtons[i])->GetValue();
							break;
						case NativeType::Float:
							GetVec<float>(Element.Variable).at(j) = std::stof(((UITextField*)ContextButtons[i])->GetText());
							break;
						case NativeType::Int:
							GetVec<int>(Element.Variable).at(j) = std::stoi(((UITextField*)ContextButtons[i])->GetText());
							break;
						case NativeType::String:
							GetVec<std::string>(Element.Variable).at(j) = ((UITextField*)ContextButtons[i])->GetText();
							break;
						case NativeType::Bool:
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
				i--;
			}
			else
			{
				try
				{
					switch (ContextSettings[IteratedElement].NativeType & ~NativeType::List)
					{
					case NativeType::Vector3Color:
					case NativeType::Vector3Rotation:
					case NativeType::Vector3:
						if (ContextSettings[IteratedElement].Normalized) 
							*(Vector3*)(ContextSettings[IteratedElement].Variable) = ((UIVectorField*)ContextButtons[i])->GetValue().Normalize();
						else
							*(Vector3*)(ContextSettings[IteratedElement].Variable) = ((UIVectorField*)ContextButtons[i])->GetValue();
						break;
					case NativeType::Float:
						*(float*)(ContextSettings[IteratedElement].Variable) = std::stof(((UITextField*)ContextButtons[i])->GetText());
						break;
					case NativeType::Int:
						*(int*)(ContextSettings[IteratedElement].Variable) = std::stoi(((UITextField*)ContextButtons[i])->GetText());
						break;
					case NativeType::String:
						*(std::string*)(ContextSettings[IteratedElement].Variable) = ((UITextField*)ContextButtons[i])->GetText();
						EditorUI::UpdateAllInstancesOf<ObjectList>();
						break;
					case NativeType::Bool:
						if (((UIButton*)ContextButtons[i])->GetIsHovered())
						{
							*(bool*)ContextSettings[IteratedElement].Variable = !(*(bool*)ContextSettings[IteratedElement].Variable);
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
		if (EditorUI::SelectedObjects.size()
			&& Index == -1)
		{
			EditorUI::SelectedObjects[0]->OnPropertySet();
		}
		EditorUI::ChangedScene = true;
		OnResized();
	}
}

void ContextMenu::OnResized()
{
	BackgroundBox->SetMinSize(Scale - Vector2(0.005f / Graphics::AspectRatio, 0.01f));
	BackgroundBox->SetMaxSize(BackgroundBox->GetMinSize());
	BackgroundBox->DeleteChildren();
	ContextSettings.clear();
	ContextButtons.clear();
	ContextCategories.clear();

	if (IsObject)
	{
		Properties.clear();
		SceneObject* SelectedObject = nullptr;
		if (!EditorUI::SelectedObjects.empty())
		{
			SelectedObject = EditorUI::SelectedObjects[0];

		}

		if (!SelectedObject)
		{
			BackgroundBox->HorizontalBoxAlign = UIBox::Align::Centered;
			BackgroundBox->AddChild((new UIText(0.5f, EditorUI::UIColors[2], "No object selected", EditorUI::Text))
				->SetPadding(0.02f)
				->SetPaddingSizeMode(UIBox::SizeMode::AspectRelative));
			return;
		}
		else
		{
			BackgroundBox->HorizontalBoxAlign = UIBox::Align::Default;
		}

		BackgroundBox->AddChild((new UIText(0.55f, EditorUI::UIColors[2], "Object: " + SelectedObject->Name, EditorUI::Text))
			->SetWrapEnabled(true, Scale.X - 0.1f, UIBox::SizeMode::ScreenRelative)
			->SetPadding(0.01f, 0, 0.01f, 0.01f));
		BackgroundBox->AddChild((new UIText(0.45f, EditorUI::UIColors[2], "Class: " + SelectedObject->GetObjectDescription().Name, EditorUI::Text))
			->SetWrapEnabled(true, Scale.X - 0.1f, UIBox::SizeMode::ScreenRelative)
			->SetPadding(0.005f, 0, 0.01f, 0.01f));
		BackgroundBox->AddChild((new UIBackground(UIBox::Orientation::Horizontal,
			0,
			EditorUI::UIColors[2],
			Vector2(Scale.X - 0.005f, 2.0f / Graphics::WindowResolution.Y)))
			->SetPadding(0, 0.005f, 0.0025f, 0));

		GenerateSection(
			{
				ContextMenuSection(&SelectedObject->GetTransform().Position, NativeType::Vector3, "Position"),
				ContextMenuSection(&SelectedObject->GetTransform().Rotation, NativeType::Vector3Rotation, "Rotation"),
				ContextMenuSection(&SelectedObject->GetTransform().Scale, NativeType::Vector3, "Scale"),
				ContextMenuSection(&SelectedObject->Name, NativeType::String, "Name"),
			},
			"Object", SelectedObject, 0);

		std::map<std::string, std::vector<ContextMenuSection>> Categories;

		for (SceneObject::Property i : SelectedObject->Properties)
		{
#ifdef ENGINE_CSHARP
			if (i.PType != SceneObject::Property::PropertyType::EditorProperty && i.PType != SceneObject::Property::PropertyType::CSharpProperty)
#else		
			if (i.PType != SceneObject::Property::PropertyType::EditorProperty)
#endif
			{
				continue;
			}
			auto Colon = i.Name.find_last_of(":");
			std::string CategoryName = SelectedObject->GetObjectDescription().Name;
			if (Colon != std::string::npos)
			{
				CategoryName = i.Name.substr(0, Colon);
				if (CategoryName.empty())
				{
					CategoryName = SelectedObject->GetObjectDescription().Name;
				}
				i.Name = i.Name.substr(Colon + 1);
			}
			if (!Categories.contains(CategoryName))
			{
				Categories.insert(std::pair(CategoryName, std::vector<ContextMenuSection>({ ContextMenuSection(i.Data, i.NativeType, i.Name) })));
			}
			else
			{
				Categories[CategoryName].push_back(ContextMenuSection(i.Data, i.NativeType, i.Name));
			}
		}

		char Iterator = 1;
		for (auto& i : Categories)
		{
			GenerateSection(i.second, i.first, SelectedObject, Iterator);
			Iterator++;
		}
	}
	else
	{
		BackgroundBox->AddChild((new UIText(0.55f, EditorUI::UIColors[2], "Scene: "
			+ FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene), EditorUI::Text))
			->SetWrapEnabled(true, Scale.X - 0.1f, UIBox::SizeMode::ScreenRelative)
			->SetPadding(0.01f, 0, 0.01f, 0.01f));
		BackgroundBox->AddChild((new UIBackground(UIBox::Orientation::Horizontal,
			0, 
			EditorUI::UIColors[2], 
			Vector2(1)))
			->SetSizeMode(UIBox::SizeMode::PixelRelative)
			->SetTryFill(true)
			->SetPadding(0, 0.005f, 0.0025f, 0));


		GenerateSection(
			{
				ContextMenuSection(&Graphics::WorldSun.Rotation, NativeType::Vector3Rotation, "Rotation"),
				ContextMenuSection(&Graphics::WorldSun.SunColor, NativeType::Vector3Color, "Color"),
				ContextMenuSection(&Graphics::WorldSun.Intensity, NativeType::Float, "Intensity")
			},
			"Sun", nullptr, 0);
		GenerateSection(
			{
				ContextMenuSection(&Graphics::WorldSun.AmbientColor, NativeType::Vector3Color, "Color"),
				ContextMenuSection(&Graphics::WorldSun.AmbientIntensity, NativeType::Float, "Intensity")
			},
			"Ambient light", nullptr, 1);
		GenerateSection(
			{
				ContextMenuSection(&Graphics::WorldFog.FogColor, NativeType::Vector3Color, "Color"),
				ContextMenuSection(&Graphics::WorldFog.Distance, NativeType::Float, "Start distance"),
				ContextMenuSection(&Graphics::WorldFog.Falloff, NativeType::Float, "Falloff"),
				ContextMenuSection(&Graphics::WorldFog.MaxDensity, NativeType::Float, "Max density")
			},
			"Fog", nullptr, 2);
		GenerateSection(
			{
				ContextMenuSection(&Graphics::MainFramebuffer->ReflectionCubemapName, NativeType::String, "Cubemap file"),
			},
			"Reflection", nullptr, 3);
	}

}

#endif