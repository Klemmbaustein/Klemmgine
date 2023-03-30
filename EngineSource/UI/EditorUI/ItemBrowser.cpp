#if EDITOR
#include "ItemBrowser.h"
#include <filesystem>
#include <UI/UIButton.h>
#include <UI/UIText.h>
#include <UI/UIScrollBox.h>
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Log.h>
#include <Engine/FileUtility.h>
#include <Engine/OS.h>
#include <Engine/Scene.h>
#include <Engine/Input.h>
#include <Math/Math.h>
#include <Objects/MeshObject.h>
#include <UI/EditorUI/Viewport.h>
#include <Objects/SoundObject.h>
#include <Objects/ParticleObject.h>
#include <stdbool.h>

#define MAX_ITEM_NAME_LENGTH 19

std::vector<ItemBrowser::FileEntry> ItemBrowser::CurrentFiles;
std::vector<UIButton*> ItemBrowser::Buttons;

std::vector<EditorClassesItem> ItemBrowser::GetEditorUIClasses()
{
	std::vector<std::string> IDs;
	EditorClassesItem RootPath;
	for (const auto& Object : Objects::EditorObjects)
	{
		// First seperate the Category into multiple names. For example: "Default/Rendering" -> { "Default", "Rendering" }
		std::string CurrentPath = Objects::GetCategoryFromID(Object.ID);
		EditorClassesItem* CurrentParent = &RootPath;
		if (CurrentPath.empty())
		{
			EditorClassesItem NewItem;
			NewItem.Name = Object.Name;
			NewItem.Object = Object;
			CurrentParent->SubItems.push_back(NewItem);
		}
		std::vector<std::string> PathElements;
		size_t Index = CurrentPath.find_first_of("/");


		while (Index != std::string::npos)
		{
			Index = CurrentPath.find_first_of("/");
			PathElements.push_back(CurrentPath.substr(0, Index));
			CurrentPath = CurrentPath.substr(Index + 1);
			Index = CurrentPath.find_first_of("/");
		}
		PathElements.push_back(CurrentPath);

		// Iterate through every 'element' we just got from the Category string
		for (const auto& elem : PathElements)
		{
			bool Found = false;
			// If that element already exists, we continue inside of it.
			for (size_t i = 0; i < CurrentParent->SubItems.size(); i++)
			{
				if (elem == CurrentParent->SubItems[i].Name)
				{
					CurrentParent = &CurrentParent->SubItems[i];
					Found = true;
					break;
				}
			}
			if (Found) continue;
			// Else we create that new element.
			EditorClassesItem NewPath;
			NewPath.IsFolder = true;
			NewPath.Name = elem;
			CurrentParent->SubItems.push_back(NewPath);
			CurrentParent = &CurrentParent->SubItems[CurrentParent->SubItems.size() - 1];
		}
		// Create a new item structure so we can add it to the folder "file system"
		EditorClassesItem NewItem;
		NewItem.Name = Object.Name;
		NewItem.Object = Object;
		CurrentParent->SubItems.push_back(NewItem);
	}

	// Debug view to display 'folder structure'
	// its very basic and only goes 3 levels deep
	// 
	/*for (const auto& i : RootPath.SubItems)
	{
		Log::Print(i.Name);
		for (const auto& j : i.SubItems)
		{
			Log::Print("   " + j.Name);
			for (const auto& k : j.SubItems)
			{
				Log::Print("      " + k.Name);
			}
		}
	}*/
	return RootPath.SubItems;
}
std::string ItemBrowser::GetCurrentCPPPathString()
{
	std::string PathString = "Classes";
	EditorClassesItem RootNode;
	RootNode.SubItems = CPPClasses;
	std::vector<EditorClassesItem> CurrentItems = RootNode.SubItems;
	for (const auto& path : CPPPath)
	{
		// Reorder the content so that folders are first and items are second.
		// It looks prettier in the item browser this way.
		std::vector<EditorClassesItem> ReordererdSubItems;
		for (const auto& i : CurrentItems[path].SubItems)
		{
			if (i.IsFolder) ReordererdSubItems.push_back(i);
		}
		for (const auto& i : CurrentItems[path].SubItems)
		{
			if (!i.IsFolder) ReordererdSubItems.push_back(i);
		}
		PathString.append("/" + CurrentItems[path].Name);
		CurrentItems = ReordererdSubItems;
	}
	PathString.append("/");
	return PathString;
}

std::vector<EditorClassesItem> ItemBrowser::GetContentsOfCurrentCPPFolder()
{
	EditorClassesItem RootNode;
	RootNode.SubItems = CPPClasses;
	std::vector<EditorClassesItem> CurrentItems = RootNode.SubItems;
	for (const auto& path : CPPPath)
	{
		// Reorder the content so that folders are first and items are second.
		// It looks prettier in the item browser this way.
		std::vector<EditorClassesItem> ReordererdSubItems;
		for (const auto& i : CurrentItems[path].SubItems)
		{
			if (i.IsFolder) ReordererdSubItems.push_back(i);
		}
		for (const auto& i : CurrentItems[path].SubItems)
		{
			if (!i.IsFolder) ReordererdSubItems.push_back(i);
		}
		CurrentItems = ReordererdSubItems;
	}
	return CurrentItems;
}

void ItemBrowser::ScanForAssets()
{
	CurrentFiles.clear();
	for (auto& entry : std::filesystem::directory_iterator(Editor::CurrentUI->CurrentPath))
	{
		if (std::filesystem::is_directory(entry))
		CurrentFiles.push_back(FileEntry(entry.path().string(), true));
	}
	for (auto& entry : std::filesystem::directory_iterator(Editor::CurrentUI->CurrentPath))
	{
		if (!std::filesystem::is_directory(entry))
		CurrentFiles.push_back(FileEntry(entry.path().string(), false));
	}
}
namespace _item
{
	size_t HoveredButton = 0;
}
void ItemBrowser::DeleteFile()
{
	std::filesystem::remove_all(CurrentFiles[_item::HoveredButton].Name);
	Editor::CurrentUI->UIElements[3]->UpdateLayout();
}

ItemBrowser::ItemBrowser(Vector3* Colors, Vector2 Position, Vector2 Scale) : EditorPanel(Colors, Position, Scale, Vector2(0.2, 1), Vector2(0.9, 4))
{
	CPPClasses = GetEditorUIClasses();
	TabBackground->SetHorizontal(false);
	TabBackground->Align = UIBox::E_REVERSE;
	ContentBox = new UIBox(false, 0);
	TabBackground->AddChild(ContentBox->SetPadding(0));

	UpdateLayout();
}

void ItemBrowser::Save()
{
}

void ItemBrowser::Load(std::string File)
{
}

void ItemBrowser::UpdateLayout()
{
	ContentBox->DeleteChildren();

	BrowserScrollBox = new UIScrollBox(false, Scale - Vector2(0, 0.1), 25);
	BrowserScrollBox->SetMinSize(Vector2(0, 1.65));
	BrowserScrollBox->SetMaxSize(Vector2(MaxSize.X, 1.65));
	BrowserScrollBox->Align = UIBox::E_REVERSE;
	ContentBox->AddChild(BrowserScrollBox
		->SetPadding(0.01, 0, 0, 0));

	ContentBox->AddChild((new UIBackground(true, 0, UIColors[1], Vector2(Scale.X, 0.1)))
		->SetPadding(0)
		->AddChild((new UIButton(true, 0, UIColors[2], this, -2))
			->SetUseTexture(true, Editor::CurrentUI->Textures[8])
			->SetMinSize(Vector2(0.06))
			->SetSizeMode(UIBox::E_PIXEL_RELATIVE)
			->SetPadding(0.01))
		->AddChild((new UITextField(true, 0, UIColors[0], this, -3, Editor::CurrentUI->EngineUIText))
			->SetText(SelectedTab == 0 ? Editor::CurrentUI->CurrentPath + "/" : GetCurrentCPPPathString())
			->SetTextSize(0.4)
			->SetMinSize(Vector2(Scale.X / 1.2 - 0.12 / Graphics::AspectRatio, 0.08))
			->SetMaxSize(Vector2(Scale.X / 1.2 - 0.12 / Graphics::AspectRatio, 0.08))
			->SetBorder(UIBox::E_ROUNDED, 0.5)
			->SetPadding(0.01)));


	auto TabBox = new UIBox(true, 0);
	size_t ButtonIndex = 0;
	ContentBox->AddChild(TabBox->SetPadding(0));
	for (auto& i : Tabs)
	{
		auto NewButton = new UIButton(true, 0, UIColors[0] * (ButtonIndex == SelectedTab ? 1.5 : 1), this, -5 - (ButtonIndex));
		NewButton->Align = UIBox::E_CENTERED;
		TabBox->AddChild(NewButton
			->SetBorder(UIBox::E_DARKENED_EDGE, 0.25)
			->SetPadding(0)
			->SetMinSize(Vector2(Scale.X / Tabs.size(), 0.06f))
			->AddChild((new UIText(0.45, UIColors[2], i, Editor::CurrentUI->EngineUIText))
				->SetPadding(0.02, -0.02, 0, 0)));
		ButtonIndex++;
	}

	ContentBox->AddChild((new UIButton(true, 0, Vector3(0.2, 0.7, 0), this, -1))
		->SetBorder(UIBox::E_ROUNDED, 0.25)
		->SetPadding(0.02, 0.02, Scale.X / 2 - 0.0575, 0.02)
		->AddChild((new UIText(0.5, 0, "Import", Editor::CurrentUI->EngineUIText))));

	ScanForAssets();
	Buttons.clear();
	const int ITEMS_PER_SLICE = Scale.X / 0.17 * Graphics::AspectRatio;
	std::vector<UIBox*> HorizontalSlices;

	std::vector<FileEntry> DisplayedFiles;

	if (SelectedTab == 0)
	{
		DisplayedFiles = CurrentFiles;
	}
	else
	{
		auto classes = GetContentsOfCurrentCPPFolder();
		for (auto& i : classes)
		{
			if (i.IsFolder)
			{
				DisplayedFiles.push_back(FileEntry(i.Name, true));
			}
			else
			{
				DisplayedFiles.push_back(FileEntry(i.Name + ".cpp", false));
			}
		}
	}

	if (!DisplayedFiles.size())
	{
		UIText* NewText = new UIText(0.4, UIColors[2], "Right click to create a new file", Editor::CurrentUI->EngineUIText);
		NewText->SetPadding(0.01);
		NewText->Wrap = true;
		NewText->WrapDistance = Scale.X * 1.2;
		BrowserScrollBox->AddChild(NewText);
	}

	HorizontalSlices.resize(DisplayedFiles.size() / ITEMS_PER_SLICE + 1);
	for (UIBox*& i : HorizontalSlices)
	{
		i = new UIBox(true, 0);
		i->SetPadding(0, 0, 0.02, 0);
		BrowserScrollBox->AddChild(i);
	}



	for (size_t i = 0; i < DisplayedFiles.size(); i++)
	{
		auto ext = FileUtil::GetExtension(DisplayedFiles[i].Name);

		if (DisplayedFiles[i].IsDirectory)
		{
			ext = "dir";
		}
		unsigned int TextureID = 4; // 4 -> X symbol
		if (Editor::ItemTextures.contains(ext))
		{
			TextureID = Editor::ItemTextures[ext];
		}
		Vector3 Color = Vector3(0.8, 0, 0);
		if (Editor::ItemColors.contains(ext))
		{
			Color = Editor::ItemColors[ext];
		}

		auto NewBackground = new UIButton(false, 0, UIColors[0] * 1.2, this, i);
		Buttons.push_back(NewBackground);
		NewBackground->SetCanBeDragged(true);
		NewBackground->SetMinSize(Vector2(0.14, 0.19));
		NewBackground->SetSizeMode(UIBox::E_PIXEL_RELATIVE);
		NewBackground->SetPadding(0.005 * Graphics::AspectRatio, 0.005 * Graphics::AspectRatio, 0.005, 0.005);
		NewBackground->Align = UIBox::E_REVERSE;
		NewBackground->SetBorder(UIBox::E_ROUNDED, 0.5);
		NewBackground->SetNeedsToBeSelected(true);
		UIBackground* ItemImage = new UIBackground(true, 0, 1, Vector2(0.12));
		ItemImage->SetSizeMode(UIBox::E_PIXEL_RELATIVE);
		ItemImage->SetUseTexture(true, Editor::CurrentUI->Textures[TextureID]);
		ItemImage->SetPadding(0);
		NewBackground->AddChild((new UIBackground(true, 0, Color, 0.1))
			->SetBorder(UIBox::E_ROUNDED, 0.5)
			->SetSizeMode(UIBox::E_PIXEL_RELATIVE)
			->SetPadding(0.0025 * Graphics::AspectRatio, 0, 0.005, 0.005)
			->AddChild(ItemImage));

		std::string ItemName = FileUtil::GetFileNameWithoutExtensionFromPath(DisplayedFiles[i].Name);

		if (ItemName.size() > MAX_ITEM_NAME_LENGTH)
		{
			ItemName = ItemName.substr(0, MAX_ITEM_NAME_LENGTH - 3).append("...");
		}

		auto ItemText = new UIText(0.3666, UIColors[2], ItemName, Editor::CurrentUI->EngineUIText);
		ItemText->Wrap = true;
		ItemText->WrapDistance = 0.1;
		ItemText->SetTextWidthOverride(0.0);
		NewBackground->AddChild(ItemText
			->SetPadding(0.0025));

		HorizontalSlices[i / ITEMS_PER_SLICE]->AddChild(NewBackground);
	}
}

void ItemBrowser::Tick()
{
	if (Input::IsRMBDown && !RMBDown && !Editor::DraggingTab && TabBackground->IsHovered() && !SelectedTab) 
	{
		RMBDown = true;
		bool IsHovered = false;
		for (size_t i = 0; i < Buttons.size(); i++)
		{
			if (Buttons[i]->GetIsHovered())
			{
				IsHovered = true;
				_item::HoveredButton = i;
			}
		}
		if (IsHovered)
		{
			Editor::CurrentUI->ShowDropdownMenu(
				{ EditorUI::DropdownItem("# " + FileUtil::GetFileNameWithoutExtensionFromPath(CurrentFiles[_item::HoveredButton].Name)),
				EditorUI::DropdownItem("Open", []()
					{
						Editor::CurrentUI->UIElements[3]->OnButtonClicked(_item::HoveredButton);
					}),
				EditorUI::DropdownItem("Rename", []()
					{
						throw "Finn issue";
					}),
				EditorUI::DropdownItem("Copy", []()
					{
						try
						{
							std::string Name = CurrentFiles[_item::HoveredButton].Name;
							std::string NewName = FileUtil::GetFilePathWithoutExtension(Name) + "_Copy." + FileUtil::GetExtension(Name);
							std::filesystem::copy(Name, NewName, std::filesystem::copy_options::recursive);
							Editor::CurrentUI->UIElements[3]->UpdateLayout();
						}
						catch (std::exception& e)
						{

						}
					}),
				EditorUI::DropdownItem("Delete", DeleteFile)
				}, Input::MouseLocation);
		}
		else
		{
			Editor::CurrentUI->ShowDropdownMenu(
				{ 
				EditorUI::DropdownItem("# Create"),
				EditorUI::DropdownItem("Folder", []()
					{
						try
						{
							std::filesystem::create_directory(Editor::CurrentUI->CurrentPath + "/Folder");
							Editor::CurrentUI->UIElements[3]->UpdateLayout();
						}
						catch (std::exception& e)
						{

						}
					}),
				EditorUI::DropdownItem("Material", []()
					{
						EditorUI::MakeEmptyFile(Editor::CurrentUI->CurrentPath + "/Material.jsmat");
						Editor::CurrentUI->UIElements[3]->UpdateLayout();
					}),
				EditorUI::DropdownItem("Material Template", []()
					{
						EditorUI::MakeEmptyFile(Editor::CurrentUI->CurrentPath + "/MaterialTemplate.jsmtmp");
						Editor::CurrentUI->UIElements[3]->UpdateLayout();
					}),
				EditorUI::DropdownItem("# Other"),
				EditorUI::DropdownItem("Import", []()
					{
						Editor::CurrentUI->UIElements[3]->OnButtonClicked(-1);
					}),
				},
				Input::MouseLocation);
		}
	}
	if (!Input::IsRMBDown)
	{
		RMBDown = false;
	}

	UpdatePanel();
	if (!Input::IsLMBDown && IsDraggingButton)
	{
		auto Viewport = Editor::CurrentUI->UIElements[4];

		IsDraggingButton--;

		if (Maths::IsPointIn2DBox(Viewport->Position, Viewport->Position + Viewport->Scale, Input::MouseLocation))
		{
			IsDraggingButton = 0;

			Vector2 RelativeMouseLocation = Input::MouseLocation - (Viewport->Position + (Viewport->Scale * 0.5));
			Vector3 Direction = Graphics::MainCamera->ForwardVectorFromScreenPosition(RelativeMouseLocation.X, RelativeMouseLocation.Y);

			Vector3 TargetSpawnLocation = Graphics::MainCamera->Position + Direction * 25;

			auto hit = Collision::LineTrace(Graphics::MainCamera->Position, Graphics::MainCamera->Position + Direction * 100);

			if (hit.Hit)
			{
				TargetSpawnLocation = hit.ImpactPoint;
			}

			for (auto i : Objects::AllObjects)
			{
				i->IsSelected = false;
			}

			std::string ext;
			if (!SelectedTab) ext = FileUtil::GetExtension(CurrentFiles[DraggedButton].Name);

			bool IsCPPClass = SelectedTab == 1;

			if (ext == "jsm" && !IsCPPClass)
			{
				auto newObject = Objects::SpawnObject<MeshObject>(Transform(TargetSpawnLocation, 0, 1));
				newObject->LoadFromFile(FileUtil::GetFileNameWithoutExtensionFromPath(CurrentFiles[DraggedButton].Name));
				newObject->SetName(FileUtil::GetFileNameWithoutExtensionFromPath(CurrentFiles[DraggedButton].Name));
				newObject->IsSelected = true;
			}
			if (ext == "jscn" && !IsCPPClass)
			{
				Scene::LoadNewScene(CurrentFiles[DraggedButton].Name);
				Scene::Tick();
				Editor::CurrentUI->UIElements[5]->UpdateLayout();
				Editor::CurrentUI->UIElements[6]->UpdateLayout();
			}
			if (ext == "wav" && !IsCPPClass)
			{
				auto newObject = Objects::SpawnObject<SoundObject>(Transform(TargetSpawnLocation, 0, 1));
				newObject->LoadSound(FileUtil::GetFileNameWithoutExtensionFromPath(CurrentFiles[DraggedButton].Name));
				newObject->IsSelected = true;
			}
			if (ext == "jspart" && !IsCPPClass)
			{
				auto newObject = Objects::SpawnObject<ParticleObject>(Transform(TargetSpawnLocation, 0, 1));
				newObject->LoadParticle(FileUtil::GetFileNameWithoutExtensionFromPath(CurrentFiles[DraggedButton].Name));
				newObject->IsSelected = true;
			}

			if (IsCPPClass)
			{
				auto newObject = Objects::SpawnObjectFromID(GetContentsOfCurrentCPPFolder()[DraggedButton].Object.ID, Transform(TargetSpawnLocation, 0, 1));
				newObject->IsSelected = true;
			}
			for (auto i : Buttons)
			{
				i->SetNeedsToBeSelected(true);
			}
			Editor::CurrentUI->UIElements[5]->UpdateLayout();

		}
	}
}

void ItemBrowser::OnButtonDragged(int Index)
{
	if (IsDraggingButton || Editor::DraggingTab)
	{
		return;
	}
	DraggedButton = Index;
	IsDraggingButton = 2;
	for (auto i : Buttons)
	{
		i->SetNeedsToBeSelected(false);
	}

	std::string ext;
	if (SelectedTab == 1)
	{
		ext = "cpp";
		if (GetContentsOfCurrentCPPFolder()[Index].IsFolder)
		{
			DraggedButton = 0;
			IsDraggingButton = 0;
			return;
		}
	}
	else
	{
		ext = FileUtil::GetExtension(CurrentFiles[Index].Name);

		if (CurrentFiles[Index].IsDirectory)
		{
			ext = "dir";
		}
	}

	unsigned int TextureID = 4; // 4 -> X symbol
	if (Editor::ItemTextures.contains(ext))
	{
		TextureID = Editor::ItemTextures[ext];
	}

	auto DragBox = new UIBackground(true, Input::MouseLocation, 1, 0.12);
	DragBox->SetSizeMode(UIBox::E_PIXEL_RELATIVE);
	DragBox->SetUseTexture(true, Editor::CurrentUI->Textures[TextureID]);

	Editor::CurrentUI->DraggedItem = DragBox;
}

void ItemBrowser::OnButtonClicked(int Index)
{
	if (Index <= -5)
	{
		SelectedTab = -Index - 5;
		UpdateLayout();
		return;
	}
	if (Index == -1)
	{
		std::string file = OS::ShowOpenFileDialog();
		if (std::filesystem::exists(file))
		{
			std::filesystem::copy(file, Editor::CurrentUI->CurrentPath + "/" + FileUtil::GetFileNameFromPath(file));
		}
		return;
	}
	if (Index == -2)
	{
		if (SelectedTab == 0)
		{
			if (IsDraggingButton)
			{
				size_t lastindex = Editor::CurrentUI->CurrentPath.find_last_of("/\\");
				std::string rawname = Editor::CurrentUI->CurrentPath.substr(0, lastindex);
				if (std::rename(CurrentFiles.at(DraggedButton).Name.c_str(),
					(rawname).append("/").append(FileUtil::GetFileNameFromPath(CurrentFiles.at(DraggedButton).Name)).c_str()) < 0)
				UpdateLayout();
				IsDraggingButton = 0;
			}
			else
			{

				auto& p = Editor::CurrentUI->CurrentPath;

				p = p.substr(0, p.find_last_of("/\\"));
			}
		}
		else if (SelectedTab == 1 && CPPPath.size())
		{
			CPPPath.pop_back();
		}

		UpdateLayout();
		return;
	}

	if (Index >= 0 && IsDraggingButton)
	{
		if (SelectedTab == 0 && CurrentFiles[Index].IsDirectory)
		{
			std::string NewName = CurrentFiles[Index].Name + "/" + FileUtil::GetFileNameFromPath(CurrentFiles[DraggedButton].Name);

			if (!std::filesystem::exists(NewName))
			{
				std::filesystem::copy(CurrentFiles[DraggedButton].Name, NewName, std::filesystem::copy_options::recursive);
				std::filesystem::remove_all(CurrentFiles[DraggedButton].Name);
				UpdateLayout();
			}
		}

		return;
	}

	// If one of the scene buttons was pressed
	if (Index >= 0 && SelectedTab == 0)
	{
		if (CurrentFiles[Index].IsDirectory)
		{
			Editor::CurrentUI->CurrentPath.append("/" + FileUtil::GetFileNameFromPath(CurrentFiles[Index].Name));
			UpdateLayout();
			return;
		}

		std::string Ext = FileUtil::GetExtension(CurrentFiles[Index].Name);

		if (Ext == "jscn")
		{
			Scene::LoadNewScene(CurrentFiles[Index].Name);
			Scene::Tick();
			Editor::CurrentUI->UIElements[5]->UpdateLayout();
			Editor::CurrentUI->UIElements[6]->UpdateLayout();
		}
		if (Ext == "png" || Ext == "jpg")
		{
			OS::OpenFile(CurrentFiles[Index].Name);
		}
		if (ExtensionTabIDs.contains(Ext))
		{
			Viewport::ViewportInstance->OpenTab(ExtensionTabIDs[Ext], CurrentFiles[Index].Name);
		}
	}
	if (Index >= 0 && SelectedTab == 1)
	{
		if (GetContentsOfCurrentCPPFolder()[Index].IsFolder)
		{
			CPPPath.push_back(Index);
			UpdateLayout();
		}
	}
}
#endif