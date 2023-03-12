#if EDITOR
#pragma once
#include <UI/EditorUI/EditorUI.h>
#include <UI/EditorUI/EditorTab.h>

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
		};

		std::vector<Setting> Settings;
	};

	size_t SelectedSetting = 0;
	TextRenderer* Renderer;
	std::vector<SettingsCategory> Preferences =
	{
		SettingsCategory("Editor", {SettingsCategory::Setting("testcategory:testsetting", Type::E_VECTOR3_COLOR, "0 0 0")}),
		SettingsCategory("Graphics")
	};

	void GenerateUI();
public:
	void OpenSettingsPage(std::string Name);

	void OnButtonClicked(int Index) override;
	PreferenceTab(Vector3* UIColors, TextRenderer* Renderer);
	void Load(std::string File) override;
	void Save() override;
};
#endif