#if EDITOR
#include "PreferenceTab.h"
#include <Engine/File/Save.h>
#include <UI/EditorUI/UIVectorField.h>
#include <Engine/Log.h>
#include <CSharp/CSharpInterop.h>
#include <filesystem>

void PreferenceTab::GenerateUI()
{
	float SegmentSize = TabBackground->GetMinSize().X - 0.5f;

	TabBackground->DeleteChildren();
	auto SettingsCategoryBox = new UIBackground(false, 0, UIColors[0] * 1.2f, Vector2(0, TabBackground->GetUsedSize().Y - 0.2f));
	SettingsCategoryBox->SetAlign(UIBox::Align::Reverse);
	TabBackground->AddChild(SettingsCategoryBox);

	for (size_t i = 0; i < Preferences.size(); i++)
	{
		SettingsCategoryBox->AddChild(
			(new UIButton(true, 0, UIColors[1] + (float)(i == SelectedSetting) / 8.f, this, (int)i))
			->SetPadding(0.005f, 0.005f, 0.01f, 0.01f)->SetBorder(UIBox::BorderType::Rounded, 0.5f)
			->AddChild((new UIText(0.5f, UIColors[2], Preferences[i].Name, Renderer))
				->SetTextWidthOverride(0.2f)
				->SetPadding(0.01f)));
	}

	auto SettingsBox = new UIBox(false, 0);
	SettingsBox->SetAlign(UIBox::Align::Reverse);
	SettingsBox->SetMinSize(Vector2(0, TabBackground->GetUsedSize().Y - 0.2f));
	TabBackground->AddChild(SettingsBox);

	SettingsBox->AddChild((new UIText(1, UIColors[2], "Settings/" + Preferences[SelectedSetting].Name, Renderer))
		->SetPadding(0, 0, 0, 0));

	SettingsBox->AddChild((new UIBackground(true, 0, UIColors[2], Vector2(SegmentSize, 0.005f)))
		->SetPadding(0, 0.1f, 0, 0));

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
	int CurentCategory = 0;
	int CurrentSettingIndex = 0;
	LoadedSettingElements.clear();
	for (auto& cat : Categories)
	{
		SettingsBox->AddChild(
			(new UIButton(true, 0, UIColors[1], this, -400 + CurentCategory))
				->SetPadding(0.01f, 0.01f, 0, 0)
				->SetMinSize(Vector2(SegmentSize, 0))
				->AddChild((new UIText(0.7f, UIColors[2], "> " + cat.first, Renderer))
					->SetPadding(0.01f)));

		for (size_t i = 0; i < cat.second.size(); i++)
		{
			try
			{
				GenerateSection(SettingsBox, cat.second[i].Name, -200 + CurrentSettingIndex, cat.second[i].Type, cat.second[i].Value);
			}
			catch (std::exception)
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
	Parent->AddChild((new UIText(0.7f, UIColors[2], Name, Renderer))->SetPadding(0.01f, 0.01f, 0.05f, 0.02f));
	UIBox* Element;
	switch (SectionType)
	{
	case Type::Float:
		break;
	case Type::Int:
		break;
	case Type::String:
		Element = (new UITextField(true, 0, UIColors[1], this, Index, Renderer))
			->SetText(Value)
			->SetMinSize(Vector2(TabBackground->GetMinSize().X - 0.6f, 0.05f))
			->SetPadding(0.02f, 0.02f, 0.05f, 0.02f)
			->SetBorder(UIBox::BorderType::Rounded, 0.5f);
		Parent->AddChild(Element);
		break;
	case Type::Vector3:
	case Type::Vector3Color:
		Element = (new UIVectorField(0, Vector3::stov(Value), this, Index, Renderer))
			->SetValueType(SectionType == Type::Vector3 ? UIVectorField::VecType::xyz : UIVectorField::VecType::rgb)
			->SetPadding(0.02f, 0.02f, 0.05f, 0.02f);
		Parent->AddChild(Element);
		break;
	case Type::Bool:
		if (Value != "0" && Value != "1")
		{
			Value = "1";
		}
		Element = (new UIButton(true, 0, 1, this, Index))
			->SetUseTexture(std::stoi(Value), Editor::CurrentUI->Textures[16])
			->SetSizeMode(UIBox::SizeMode::PixelRelative)
			->SetMinSize(0.05f)
			->SetPadding(0.02f, 0.02f, 0.05f, 0.02f)
			->SetBorder(UIBox::BorderType::Rounded, 0.5f);
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
		case Type::Bool:
		{
			bool Val = (bool)std::stoi(Setting.Value);
			Setting.Value = std::to_string(!Val);
		}
			break;
		case Type::String:
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

#if ENGINE_CSHARP
	if (!CSharp::GetUseCSharp())
	{
		Toolbar::ToolbarInstance->SetButtonVisibility("Reload C#", false);
	}
#endif
}

void PreferenceTab::Load(std::string File)
{
	SaveGame Pref = SaveGame("../../EditorContent/Config/EditorPrefs", "pref", false);
	SaveGame Proj = SaveGame(Build::GetProjectBuildName(), "keproj", false);
	for (auto& cat : Preferences)
	{
		SaveGame& UsedSave = cat.Name == "Project specific" ? Proj : Pref;
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
			if (UsedSave.GetProperty(NameCopy).Type != Type::Null)
			{
				i.Value = UsedSave.GetProperty(NameCopy).Value;
				i.OnChanged(i.Value);
			}
		}
	}
}

void PreferenceTab::Save()
{
	std::filesystem::create_directories("../../EditorContent/Config");
	SaveGame Pref = SaveGame("../../EditorContent/Config/EditorPrefs", "pref", false);
	SaveGame Proj = SaveGame(Build::GetProjectBuildName(), "keproj", false);
	for (auto& cat : Preferences)
	{
		SaveGame& UsedSave = cat.Name == "Project specific" ? Proj : Pref;
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
			UsedSave.SetProperty(SaveGame::SaveProperty(i.Name, i.Value, i.Type));
		}
	}
}
#endif