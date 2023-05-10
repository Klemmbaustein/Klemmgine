#if EDITOR
#include "RenameBox.h"
#include <UI/EditorUI/EditorUI.h>
#include <Engine/FileUtility.h>
#include <filesystem>

RenameBox::RenameBox(std::string FileToRename, Vector2 Position) 
	: EditorPanel(Editor::CurrentUI->UIColors, Position, Vector2(0.4, 0.2), Vector2(0.3, 0.2), 2, true,
		"Rename \"" + FileUtil::GetFileNameWithoutExtensionFromPath(FileToRename) + "\"")
{
	File = FileToRename;

	ButtonBackground = new UIBackground(true, 0, UIColors[0] * 1.5);
	ButtonBackground->SetPadding(0);
	ButtonBackground->SetBorder(UIBox::E_DARKENED_EDGE, 0.2);
	TabBackground->Align = UIBox::E_DEFAULT;
	TabBackground->AddChild(ButtonBackground);

	ButtonBackground->AddChild((new UIButton(true, 0, UIColors[2], this, -2))
		->SetPadding(0.01)
		->SetBorder(UIBox::E_ROUNDED, 0.2)
		->AddChild((new UIText(0.45, 1 - UIColors[2], "Confirm", Editor::CurrentUI->EngineUIText))
			->SetPadding(0.005)))
	->AddChild((new UIButton(true, 0, UIColors[2], this, -1))
		->SetPadding(0.01)
		->SetBorder(UIBox::E_ROUNDED, 0.2)
		->AddChild((new UIText(0.45, 1 - UIColors[2], "Cancel", Editor::CurrentUI->EngineUIText))
			->SetPadding(0.005)));

	InputField = new UITextField(true, 0, UIColors[1], this, 0, Editor::CurrentUI->EngineUIText);

	TabBackground->AddChild((new UIBox(true, 0))
		->SetPadding(0)
		->AddChild((new UIText(0.4, UIColors[2], "To: ", Editor::CurrentUI->EngineUIText))
			->SetPadding(0.01, 0.01, 0.02, 0))
		->AddChild(InputField
			->SetText(FileUtil::GetFileNameWithoutExtensionFromPath(FileToRename))
			->SetTextSize(0.4)
			->SetPadding(0.01, 0.01, 0, 0)
			->SetMinSize(Vector2(0.2, 0.01)))
		->AddChild((new UIText(0.4, UIColors[2], "." + FileUtil::GetExtension(FileToRename), Editor::CurrentUI->EngineUIText))
			->SetPadding(0.01, 0.015, 0, 0)));

	TabBackground->AddChild(new UIText(0.4, UIColors[2], "From:  " + FileUtil::GetFileNameFromPath(FileToRename), Editor::CurrentUI->EngineUIText));
	UpdateLayout();
}

void RenameBox::UpdateLayout()
{
	ButtonBackground->SetMinSize(Vector2(TabBackground->GetMinSize().X, 0.075));
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
		Editor::CurrentUI->UIElements[3]->UpdateLayout();
		delete this;
		return;
	}
}

void RenameBox::Tick()
{
	UpdatePanel();
}
#endif