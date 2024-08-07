#if EDITOR
#include "SettingsPanel.h"
#include <Engine/File/SaveData.h>
#include <UI/EditorUI/UIVectorField.h>
#include <Engine/Log.h>
#include <Engine/Subsystem/CSharpInterop.h>
#include <filesystem>
#include <UI/UIScrollBox.h>
#include "Viewport.h"

void SettingsPanel::GenerateUI()
{
	float SegmentSize = Scale.X - 0.25f;

	if (!SettingsBox)
	{
		SettingsCategoryBox = new UIBackground(UIBox::Orientation::Vertical, 0, EditorUI::UIColors[0] * 1.2f, Vector2(0, Scale.Y - 0.2f));
		HorizontalBox->AddChild(SettingsCategoryBox
			->SetPadding(0.0025f));

		SettingsBox = new UIScrollBox(UIBox::Orientation::Vertical, 0, true);
		HorizontalBox->AddChild(SettingsBox
			->SetPadding(0.0025f));
	}
	SettingsBox->DeleteChildren();
	SettingsCategoryBox->DeleteChildren();

	for (size_t i = 0; i < Preferences.size(); i++)
	{
		if (Preferences[i].Name == "CMake" && !Build::CMake::IsUsingCMake())
		{
			continue;
		}

		UIButton* CategoryButton = (new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[1], this, (int)i));

		if (SelectedSetting == i)
		{
			CategoryButton->AddChild((new UIBackground(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2], 10))
				->SetSizeMode(UIBox::SizeMode::PixelRelative)
				->SetTryFill(true));
		}

		SettingsCategoryBox->AddChild(CategoryButton
			->SetMinSize(Vector2(0.2f, 0))
			->SetPadding(0.005f, 0.005f, 0.01f, 0.01f)
			->AddChild((new UIText(0.45f, EditorUI::UIColors[2], Preferences[i].Name, EditorUI::Text))
				->SetPadding(0.01f)));
	}

	SettingsBox->SetMinSize(Vector2(Scale.X - 0.25f, Scale.Y - 0.005f));
	SettingsBox->SetMaxSize(Vector2(Scale.X - 0.25f, Scale.Y - 0.005f));
	SettingsCategoryBox->SetMinSize(Vector2(0, Scale.Y - 0.005f));
	SettingsCategoryBox->SetMaxSize(Vector2(2, Scale.Y - 0.005f));
	SettingsCategoryBox->SetColor(EditorUI::UIColors[0] * 1.2f);

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
			(new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[1], this, -400 + CurentCategory))
				->SetPadding(0.01f, 0.01f, 0, 0)
				->SetMinSize(Vector2(SegmentSize, 0))
				->AddChild((new UIText(0.5f, EditorUI::UIColors[2], "> " + cat.first, EditorUI::Text))
					->SetPadding(0.01f)));

		for (size_t i = 0; i < cat.second.size(); i++)
		{
			try
			{
				GenerateSection(SettingsBox, cat.second[i].Name, -200 + CurrentSettingIndex, cat.second[i].NativeType, cat.second[i].Value);
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

void SettingsPanel::GenerateSection(UIBox* Parent, std::string Name, int Index, NativeType::NativeType SectionType, std::string Value)
{
	float Size = std::min(PanelMainBackground->GetUsedSize().X - 0.5f, 0.5f);

	Parent->AddChild((new UIText(0.45f, EditorUI::UIColors[2], Name, EditorUI::Text))->SetPadding(0.01f, 0.01f, 0.05f, 0.02f));
	UIBox* Element;
	switch (SectionType)
	{
	case NativeType::Float:
	case NativeType::Int:
	case NativeType::String:
		Element = (new UITextField(0, EditorUI::UIColors[1], this, Index, EditorUI::Text))
			->SetText(Value)
			->SetMinSize(Vector2(Size, 0))
			->SetPadding(0.01f, 0.02f, 0.05f, 0.02f);
		Parent->AddChild(Element);
		break;
	case NativeType::Vector3:
	case NativeType::Vector3Color:
		Element = (new UIVectorField(Size, Vector3::FromString(Value), this, Index, EditorUI::Text))
			->SetValueType(SectionType == NativeType::Vector3 ? UIVectorField::VecType::xyz : UIVectorField::VecType::rgb)
			->SetPadding(0.01f, 0.02f, 0.05f, 0.02f);
		Parent->AddChild(Element);
		break;
	case NativeType::Bool:
		if (Value != "0" && Value != "1")
		{
			Value = "0";
		}
		Element = (new UIButton(UIBox::Orientation::Horizontal, 0, 1, this, Index))
			->SetUseTexture(std::stoi(Value), Application::EditorInstance->Textures[16])
			->SetSizeMode(UIBox::SizeMode::AspectRelative)
			->SetMinSize(0.04f)
			->SetPadding(0.01f, 0.02f, 0.05f, 0.02f)
			->SetBorder(UIBox::BorderType::Rounded, 0.5f);
		Parent->AddChild(Element);
		break;
	default:
		break;
	}
	LoadedSettingElements.push_back(Element);
}

void SettingsPanel::NewSettingsPanel()
{
	auto NewTab = new SettingsPanel(nullptr);
	Viewport::ViewportInstance->AddPanelTab(NewTab);
}

void SettingsPanel::OpenSettingsPage(std::string Name)
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

void SettingsPanel::OnResized()
{
	GenerateUI();
}

void SettingsPanel::OnButtonClicked(int Index)
{
	if (Index < -1000)
	{
		HandlePanelButtons(Index);
		return;
	}
	if (Index >= 0)
	{
		SelectedSetting = Index;
		GenerateUI();
	}
	else if (Index >= -200)
	{
		Index += 200;
		auto& Setting = Preferences[LoadedSettings.at(Index).cat].Settings[LoadedSettings.at(Index).entry];
		switch (Setting.NativeType)
		{
		case NativeType::Bool:
		{
			bool Val = false;
			try
			{
				Val = (bool)std::stoi(Setting.Value);
			}
			catch (std::exception)
			{

			}
			Setting.Value = std::to_string(!Val);
		}
			break;
		case NativeType::String:
			Setting.Value = dynamic_cast<UITextField*>(LoadedSettingElements[Index])->GetText();
			break;
		case NativeType::Int:
		{
			try
			{
				Setting.Value = std::to_string(std::stoi(dynamic_cast<UITextField*>(LoadedSettingElements[Index])->GetText()));
			}
			catch (std::exception)
			{

			}
			break;
		}
		case NativeType::Float:
		{
			try
			{
				Setting.Value = std::to_string(std::stof(dynamic_cast<UITextField*>(LoadedSettingElements[Index])->GetText()));
			}
			catch (std::exception)
			{

			}
			break;
		}
		default:
			break;
		}
		Setting.OnChanged(Setting.Value);

		OnResized();
	}
	Save();
}

SettingsPanel::SettingsPanel(EditorPanel* Parent) : EditorPanel(Parent, "Settings", "settings")
{
	CanBeClosed = true;
	HorizontalBox = new UIBox(UIBox::Orientation::Horizontal, 0);
	PanelMainBackground->AddChild(HorizontalBox);

	GenerateUI();
	Load();

#if ENGINE_CSHARP
	if (!CSharpInterop::GetUseCSharp())
	{
		Toolbar::ToolbarInstance->SetButtonVisibility("Reload C#", false);
	}
#endif
}

void SettingsPanel::Load()
{
	SaveData Pref = SaveData(Application::GetEditorPath() + "/EditorContent/Config/EditorPrefs", "pref", false);
	SaveData Proj = SaveData(Build::GetProjectBuildName(), "keproj", false);
	for (auto& cat : Preferences)
	{
		SaveData& UsedSave = cat.Name == "Project specific" ? Proj : Pref;
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
			if (UsedSave.HasField(NameCopy))
			{
				i.Value = UsedSave.GetField(NameCopy).Data;
				try
				{
					i.OnChanged(i.Value);
				}
				catch (std::exception)
				{
				}
			}
		}
	}
}

void SettingsPanel::Save()
{
	std::filesystem::create_directories(Application::GetEditorPath() + "/EditorContent/Config");
	SaveData Pref = SaveData(Application::GetEditorPath() + "/EditorContent/Config/EditorPrefs", "pref", false);
	SaveData Proj = SaveData(Build::GetProjectBuildName(), "keproj", false);
	for (auto& cat : Preferences)
	{
		SaveData& UsedSave = cat.Name == "Project specific" ? Proj : Pref;
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
			UsedSave.SetField(SaveData::Field(i.NativeType, i.Name, i.Value));
		}
	}
}
#endif