#if EDITOR
#include "RenameBox.h"
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Utility/FileUtility.h>
#include <filesystem>
#include <Engine/Input.h>
#include <UI/EditorUI/AssetBrowser.h>

RenameBox::RenameBox(std::string FileToRename) 
	: EditorPopup(0, Vector2(0.4f, 0.2f),
		"Rename \"" + FileUtil::GetFileNameWithoutExtensionFromPath(FileToRename) + "\"")
{
	File = FileToRename;

	SetOptions({ PopupOption("Confirm"), PopupOption("Cancel") });

	InputField = new UITextField(0, EditorUI::UIColors[1], this, -1, EditorUI::Text);

	std::string Ext = FileUtil::GetExtension(FileToRename);
	PopupBackground->AddChild(new UIText(0.4f, EditorUI::UIColors[2], "From:  " + FileUtil::GetFileNameFromPath(FileToRename), EditorUI::Text));
	PopupBackground->AddChild((new UIBox(UIBox::Orientation::Horizontal, 0))
		->SetPadding(0)
		->AddChild((new UIText(0.4f, EditorUI::UIColors[2], "To:   ", EditorUI::Text))
			->SetPadding(0.01f, 0.01f, 0.02f, 0))
		->AddChild(InputField
			->SetText(FileUtil::GetFileNameWithoutExtensionFromPath(FileToRename))
			->SetTextSize(0.4f)
			->SetPadding(0.01f, 0.01f, 0, 0)
			->SetMinSize(Vector2(0.2f, 0.01f)))
		->AddChild((new UIText(0.4f, EditorUI::UIColors[2], Ext.empty() ? "" : "." + FileUtil::GetExtension(FileToRename), EditorUI::Text))
			->SetPadding(0.01f, 0.015f, 0, 0)));

	InputField->Edit();
}


RenameBox::~RenameBox()
{
}

void RenameBox::OnButtonClicked(int Index)
{
	if (Index == 1)
	{
		delete this;
		return;
	}
	if (Index == 0)
	{
		std::string Path = File.substr(0, File.find_last_of("/\\"));
		std::filesystem::rename(File, Path + "/" + InputField->GetText() + "." + FileUtil::GetExtension(File));
		AssetBrowser::UpdateAll();
		delete this;
		return;
	}
}

void RenameBox::Tick()
{
	TickPopup();

	if (Input::IsKeyDown(Input::Key::RETURN))
	{
		OnButtonClicked(0);
	}
}
#endif