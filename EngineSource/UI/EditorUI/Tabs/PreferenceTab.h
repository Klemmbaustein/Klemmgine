#if EDITOR
#pragma once
#include <UI/EditorUI/EditorUI.h>
#include <UI/EditorUI/Tabs/EditorTab.h>
#include <UI/EditorUI/Toolbar.h>
#include <Engine/Log.h>

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
			size_t cat = 0, entry = 0;
		};

		std::vector<Setting> Settings;
	};

	std::vector<UIBox*> LoadedSettingElements;

	size_t SelectedSetting = 0;
	TextRenderer* Renderer;

	std::vector<SettingsCategory::Setting> LoadedSettings;

	std::vector<SettingsCategory> Preferences =
	{
		SettingsCategory("Editor", 
			{
			SettingsCategory::Setting("UI:Light mode [Experimental]", Type::E_BOOL, "0", [](std::string NewValue)
			{
				Editor::CurrentUI->SetUseLightMode(std::stoi(NewValue));
			}),
			SettingsCategory::Setting("Toolbar:Show Save Button", Type::E_BOOL, "1", [](std::string NewValue)
			{
				Toolbar::ToolbarInstance->SetButtonVisibility("Save", std::stoi(NewValue));
			}),
			SettingsCategory::Setting("Toolbar:Show Wireframe Button", Type::E_BOOL, "1", [](std::string NewValue)
			{	
				Toolbar::ToolbarInstance->SetButtonVisibility("Wireframe", std::stoi(NewValue));
			}),
			SettingsCategory::Setting("Toolbar:Show Build Button", Type::E_BOOL, "1", [](std::string NewValue)
			{
				Toolbar::ToolbarInstance->SetButtonVisibility("Build", std::stoi(NewValue));
			}),
#ifdef ENGINE_CSHARP
			SettingsCategory::Setting("Toolbar:Show run button", Type::E_BOOL, "1", [](std::string NewValue)
			{
				Toolbar::ToolbarInstance->SetButtonVisibility("Run", std::stoi(NewValue));
			})
#endif
			}
		),
#ifdef ENGINE_CSHARP
		SettingsCategory("Klemmgine.NET",
			{
				SettingsCategory::Setting("Run from editor:Launch arguments", Type::E_STRING, "-neverhideconsole", [](std::string NewValue)
				{
					EditorUI::LaunchInEditorArgs = NewValue;
				})
			}
		),

#endif
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