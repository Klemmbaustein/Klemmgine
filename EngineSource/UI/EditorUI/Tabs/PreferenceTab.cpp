#if EDITOR
#include "PreferenceTab.h"
#include <Engine/Save.h>
#include <UI/EditorUI/UIVectorField.h>
#include <Engine/Log.h>

void PreferenceTab::GenerateUI()
{
	float SegmentSize = TabBackground->GetMinSize().X - 0.5;

	TabBackground->DeleteChildren();
	auto SettingsCategoryBox = new UIBackground(false, 0, UIColors[0] * 1.2, Vector2(0, TabBackground->GetUsedSize().Y - 0.2));
	SettingsCategoryBox->Align = UIBox::E_REVERSE;
	TabBackground->AddChild(SettingsCategoryBox);

	for (size_t i = 0; i < Preferences.size(); i++)
	{
		SettingsCategoryBox->AddChild(
			(new UIButton(true, 0, UIColors[1] + (float)(i == SelectedSetting) / 8.f, this, i))->
			SetPadding(0.005, 0.005, 0.01, 0.01)->SetBorder(UIBox::E_ROUNDED, 0.5)->
			AddChild((new UIText(0.5, UIColors[2], Preferences[i].Name, Renderer))->SetTextWidthOverride(0.2)->SetPadding(0.01)));
	}

	auto SettingsBox = new UIBox(false, 0);
	SettingsBox->Align = UIBox::E_REVERSE;
	SettingsBox->SetMinSize(Vector2(0, TabBackground->GetUsedSize().Y - 0.2));
	TabBackground->AddChild(SettingsBox);

	SettingsBox->AddChild((new UIText(1, UIColors[2], "Settings/" + Preferences[SelectedSetting].Name, Renderer))
		->SetPadding(0, 0, 0, 0));

	SettingsBox->AddChild((new UIBackground(true, 0, UIColors[2], Vector2(SegmentSize, 0.005)))->SetPadding(0, 0.1, 0, 0));

	std::map<std::string, std::vector<SettingsCategory::Setting>> Categories;

	// Copy the array so it can be modifed.

	size_t iter = 0;
	auto SettingsArray = Preferences[SelectedSetting].Settings;
	for (auto& i : SettingsArray)
	{
		i.entry = iter++;
		i.cat = SelectedSetting;
		auto Colon = i.Name.find_last_of(":");
		std::string CategoryName = "No category";
		if (Colon != std::string::npos)
		{
			CategoryName = i.Name.substr(0, Colon);
			i.Name = i.Name.substr(Colon + 1);
		}
		if (!Categories.contains(CategoryName))
		{
			Categories.insert(std::pair(CategoryName, std::vector<SettingsCategory::Setting>({ i })));
		}
		else
		{
			Categories[CategoryName].push_back(i);
		}
	}
	LoadedSettings.clear();
	size_t CurentCategory = 0;
	size_t CurrentSettingIndex = 0;
	LoadedSettingElements.clear();
	for (auto& cat : Categories)
	{
		SettingsBox->AddChild(
			(new UIButton(true, 0, UIColors[1], this, -400 + CurentCategory))
				->SetPadding(0.01, 0.01, 0, 0)
				->SetMinSize(Vector2(SegmentSize, 0))
				->AddChild((new UIText(0.7, UIColors[2], "> " + cat.first, Renderer))
					->SetPadding(0.01)));

		for (size_t i = 0; i < cat.second.size(); i++)
		{
			try
			{
				GenerateSection(SettingsBox, cat.second[i].Name, -200 + CurrentSettingIndex, cat.second[i].Type, cat.second[i].Value);
			}
			catch (std::exception& e)
			{
				cat.second[i].Value = "0";
			}
			LoadedSettings.push_back(cat.second[i]); 
			CurrentSettingIndex++;
		}
		CurentCategory++;
	}
}

void PreferenceTab::GenerateSection(UIBox* Parent, std::string Name, int Index, Type::TypeEnum SectionType, std::string Value)
{
	Parent->AddChild((new UIText(0.7, UIColors[2], Name, Renderer))->SetPadding(0.01, 0.01, 0.05, 0.02));
	UIBox* Element;
	switch (SectionType)
	{
	case Type::E_FLOAT:
		break;
	case Type::E_INT:
		break;
	case Type::E_STRING:
		Element = (new UITextField(true, 0, UIColors[1], this, Index, Renderer))
			->SetText(Value)
			->SetMinSize(Vector2(0.8, 0.05))
			->SetPadding(0.02, 0.02, 0.05, 0.02)
			->SetBorder(UIBox::E_ROUNDED, 0.5);
		Parent->AddChild(Element);
		break;
	case Type::E_VECTOR3:
	case Type::E_VECTOR3_COLOR:
		Element = (new UIVectorField(0, Vector3::stov(Value), this, Index, Renderer))
			->SetValueType(SectionType == Type::E_VECTOR3 ? UIVectorField::E_XYZ : UIVectorField::E_RGB)
			->SetPadding(0.02, 0.02, 0.05, 0.02);
		Parent->AddChild(Element);
		break;
	case Type::E_BOOL:
		Element = (new UIButton(true, 0, 1, this, Index))
			->SetUseTexture(std::stoi(Value), Editor::CurrentUI->Textures[16])
			->SetSizeMode(UIBox::E_PIXEL_RELATIVE)
			->SetMinSize(0.05)
			->SetPadding(0.02, 0.02, 0.05, 0.02)
			->SetBorder(UIBox::E_ROUNDED, 0.5);
		Parent->AddChild(Element);
		break;
	default:
		break;
	}
	LoadedSettingElements.push_back(Element);
}

void PreferenceTab::OpenSettingsPage(std::string Name)
{
	for (size_t i = 0; i < Preferences.size(); i++)
	{
		if (Preferences[i].Name == Name)
		{
			SelectedSetting = i;
			GenerateUI();
			return;
		}
	}
}

void PreferenceTab::UpdateLayout()
{
	GenerateUI();
}

void PreferenceTab::OnButtonClicked(int Index)
{
	if (Index >= 0)
	{
		SelectedSetting = Index;
		GenerateUI();
	}
	else if (Index >= -200)
	{
		Index += 200;
		auto& Setting = Preferences[LoadedSettings.at(Index).cat].Settings[LoadedSettings.at(Index).entry];
		switch (Setting.Type)
		{
		case Type::E_BOOL:
			Setting.Value = std::to_string(!std::stoi(Setting.Value));
			break;
		case Type::E_STRING:
			Setting.Value = dynamic_cast<UITextField*>(LoadedSettingElements[Index])->GetText();
			break;
		default:
			break;
		}
		Setting.OnChanged(Setting.Value);

		UpdateLayout();
	}
}

PreferenceTab::PreferenceTab(Vector3* UIColors, TextRenderer* Renderer) : EditorTab(UIColors)
{
	this->Renderer = Renderer;
	TabBackground->SetHorizontal(true);
	GenerateUI();
}

void PreferenceTab::Load(std::string File)
{
	SaveGame Pref = SaveGame("../../EditorContent/Config/EditorPrefs", "pref", false);
	for (auto& cat : Preferences)
	{
		for (auto& i : cat.Settings)
		{
			std::string NameCopy = i.Name;
			while (true)
			{
				auto Space = NameCopy.find_first_of(" ");
				if (Space == std::string::npos)
				{
					break;
				}
				NameCopy[Space] = '_';
			}
			if (Pref.GetPropterty(NameCopy).Type != Type::E_NULL)
			{
				i.Value = Pref.GetPropterty(NameCopy).Value;
				i.OnChanged(i.Value);
			}
		}
	}
}

void PreferenceTab::Save()
{
	SaveGame Pref = SaveGame("../../EditorContent/Config/EditorPrefs", "pref", false);
	for (auto& cat : Preferences)
	{
		for (auto i : cat.Settings)
		{
			while (true)
			{
				auto Space = i.Name.find_first_of(" ");
				if (Space == std::string::npos)
				{
					break;
				}
				i.Name[Space] = '_';
			}
			Pref.SetPropterty(SaveGame::SaveProperty(i.Name, i.Value, i.Type));
		}
	}
}
#endif