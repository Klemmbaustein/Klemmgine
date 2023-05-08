#if EDITOR
#pragma once
#include <UI/EditorUI/EditorUI.h>
#include <UI/EditorUI/Tabs/EditorTab.h>

class PreferenceTab : public EditorTab
{
	struct SettingsCategory
	{
		std::string Name;

		struct Setting
		{
			std::string Name;
			Type::TypeEnum Type;
			std::string Value;
			void (*OnChanged)(std::string NewValue);
		};

		std::vector<Setting> Settings;
	};

	size_t SelectedSetting = 0;
	TextRenderer* Renderer;
	std::vector<SettingsCategory> Preferences =
	{
		SettingsCategory("Editor", {SettingsCategory::Setting("UI:Light mode [Experimental]", Type::E_BOOL, "0", [](std::string NewValue)
			{
				Editor::CurrentUI->SetUseLightMode(std::stoi(NewValue));
			})}),
	};

	void GenerateUI();
	void GenerateSection(UIBox* Parent, std::string Name, int Index, Type::TypeEnum SectionType, std::string Value);
public:
	void OpenSettingsPage(std::string Name);

	void UpdateLayout() override;

	void OnButtonClicked(int Index) override;
	PreferenceTab(Vector3* UIColors, TextRenderer* Renderer);
	void Load(std::string File) override;
	void Save() override;
};
#endif