#if EDITOR && 0
#include "RenameBox.h"
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Utility/FileUtility.h>
#include <filesystem>
#include <Engine/Input.h>

RenameBox::RenameBox(std::string FileToRename, Vector2 Position) 
	: EditorPanel(Application::EditorInstance->UIColors, Position, Vector2(0.4f, 0.2f), Vector2(0.3f, 0.2f), 2, true,
		"Rename \"" + FileUtil::GetFileNameWithoutExtensionFromPath(FileToRename) + "\"")
{
	File = FileToRename;
	ButtonBackground = new UIBackground(true, 0, UIColors[0] * 1.5f);
	ButtonBackground->SetPadding(0);
	ButtonBackground->SetVerticalAlign(UIBox::Align::Centered);
	ButtonBackground->SetBorder(UIBox::BorderType::DarkenedEdge, 0.2f);
	TabBackground->SetVerticalAlign(UIBox::Align::Default);
	TabBackground->AddChild(ButtonBackground);

	ButtonBackground->AddChild((new UIButton(true, 0, UIColors[2], this, -2))
		->SetPadding(0.01f)
		->SetBorder(UIBox::BorderType::Rounded, 0.2f)
		->AddChild((new UIText(0.45f, 1 - UIColors[2], "Confirm", Application::EditorInstance->EngineUIText))
			->SetPadding(0.005f)))
	->AddChild((new UIButton(true, 0, UIColors[2], this, -1))
		->SetPadding(0.01f)
		->SetBorder(UIBox::BorderType::Rounded, 0.2f)
		->AddChild((new UIText(0.45f, 1 - UIColors[2], "Cancel", Application::EditorInstance->EngineUIText))
			->SetPadding(0.005f)));

	InputField = new UITextField(0, UIColors[1], this, 0, Application::EditorInstance->EngineUIText);

	std::string Ext = FileUtil::GetExtension(FileToRename);

	TabBackground->AddChild((new UIBox(true, 0))
		->SetPadding(0)
		->AddChild((new UIText(0.4f, UIColors[2], "To:   ", Application::EditorInstance->EngineUIText))
			->SetPadding(0.01f, 0.01f, 0.02f, 0))
		->AddChild(InputField
			->SetText(FileUtil::GetFileNameWithoutExtensionFromPath(FileToRename))
			->SetTextSize(0.4f)
			->SetPadding(0.01f, 0.01f, 0, 0)
			->SetMinSize(Vector2(0.2f, 0.01f)))
		->AddChild((new UIText(0.4f, UIColors[2], Ext.empty() ? "" : "." + FileUtil::GetExtension(FileToRename), Application::EditorInstance->EngineUIText))
			->SetPadding(0.01f, 0.015f, 0, 0)));

	InputField->Edit();

	TabBackground->AddChild(new UIText(0.4f, UIColors[2], "From:  " + FileUtil::GetFileNameFromPath(FileToRename), Application::EditorInstance->EngineUIText));
	UpdateLayout();
}

void RenameBox::UpdateLayout()
{
	ButtonBackground->SetMinSize(Vector2(TabBackground->GetMinSize().X, 0.075f));
}

RenameBox::~RenameBox()
{
}

void RenameBox::OnButtonClicked(int Index)
{
	if (Index == -1)
	{
		delete this;
		return;
	}
	if (Index == -2)
	{
		std::string Path = File.substr(0, File.find_last_of("/\\"));
		std::filesystem::rename(File, Path + "/" + InputField->GetText() + "." + FileUtil::GetExtension(File));
		Application::EditorInstance->UIElements[3]->UpdateLayout();
		delete this;
		return;
	}
}

void RenameBox::Tick()
{
	UpdatePanel();
	if (Input::IsKeyDown(Input::Key::RETURN))
	{
		OnButtonClicked(-2);
	}
}
#endif