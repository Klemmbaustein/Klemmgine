#if EDITOR
#include "PreferenceTab.h"
#include <Engine/Save.h>

void PreferenceTab::GenerateUI()
{
	TabBackground->DeleteChildren();
	auto SettingsCategoryBox = new UIBackground(false, 0, UIColors[0] * 1.2, Vector2(0, 1.2));
	SettingsCategoryBox->Align = UIBox::E_REVERSE;
	TabBackground->AddChild(SettingsCategoryBox);

	for (size_t i = 0; i < Preferences.size(); i++)
	{
		SettingsCategoryBox->AddChild(
			(new UIButton(true, 0, UIColors[2] * 0.25 + (float)(i == SelectedSetting) / 6.f, this, i))->
			SetPadding(0.005, 0.005, 0.01, 0.01)->SetBorder(UIBox::E_ROUNDED, 0.5)->
			AddChild((new UIText(0.5, 1, Preferences[i].Name, Renderer))->SetTextWidthOverride(0.2)->SetPadding(0.01)));
	}

	auto SettingsBox = new UIBox(false, 0);
	SettingsBox->Align = UIBox::E_REVERSE;
	SettingsBox->SetMinSize(Vector2(0, 1.2));
	TabBackground->AddChild(SettingsBox);

	SettingsBox->AddChild((new UIText(1, 1, Preferences[SelectedSetting].Name, Renderer)));

	std::map<std::string, std::vector<SettingsCategory::Setting>> Categories;

	// Copy the array so it can be modifed.
	auto SettingsArray = Preferences[SelectedSetting].Settings;
	for (auto& i : SettingsArray)
	{
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

	size_t CurentCategory = 0;
	for (auto& cat : Categories)
	{
		SettingsBox->AddChild(
			(new UIButton(true, 0, UIColors[2] * 0.4, this, -400 + CurentCategory))->SetBorder(UIBox::E_ROUNDED, 0.5)->SetPadding(0.005, 0.005, 0.01, 0.01)->
			AddChild((new UIText(0.5, 1, cat.first, Renderer))->SetTextWidthOverride(0.42)->SetPadding(0.01)));

		for (size_t i = 0; i < cat.second.size(); i++)
		{
			SettingsBox->AddChild(
				(new UIButton(true, 0, UIColors[2] * 0.2, this, -200 + i))->SetBorder(UIBox::E_ROUNDED, 0.5)->SetPadding(0.005, 0.005, 0.03, 0.01)->
				AddChild((new UIText(0.4, 1, cat.second[i].Name, Renderer))->SetTextWidthOverride(0.4)->SetPadding(0.01)));
		}
		CurentCategory++;
	}
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

void PreferenceTab::OnButtonClicked(int Index)
{
	if (Index >= 0)
	{
		SelectedSetting = Index;
		GenerateUI();
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
}

void PreferenceTab::Save()
{
	SaveGame Pref = SaveGame("EditorContent/Config/EditorPrefs", "pref", false);
	Pref.SetPropterty(SaveGame::SaveProperty("test", "hi", Type::E_STRING));
}
#endif