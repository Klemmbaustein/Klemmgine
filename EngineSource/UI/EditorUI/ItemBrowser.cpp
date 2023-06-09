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
#include <Engine/Importers/Importer.h>
#include <Engine/Importers/ModelConverter.h>
#include <UI/EditorUI/Popups/RenameBox.h>
#include <World/Assets.h>
#ifdef ENGINE_CSHARP
#include <CSharp/CSharpInterop.h>
#include <Objects/CSharpObject.h>
#endif

#define MAX_ITEM_NAME_LENGTH 19

std::vector<ItemBrowser::FileEntry> ItemBrowser::CurrentFiles;
std::vector<UIButton*> ItemBrowser::Buttons;
size_t ItemBrowser::SelectedButton = 0;


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
			continue;
		}
		std::vector<std::string> PathElements;
		size_t Index = std::string::npos;

		do
		{
			Index = CurrentPath.find_first_of("/");
			PathElements.push_back(CurrentPath.substr(0, Index));
			CurrentPath = CurrentPath.substr(Index + 1);
		} while (Index != std::string::npos);

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

#ifdef ENGINE_CSHARP
	for (auto& i : CSharp::GetAllClasses())
	{
		EditorClassesItem NewItem;
		NewItem.Name = i;
		NewItem.Object = ObjectDescription(i, CSharpObject::GetID());
		RootPath.SubItems.push_back(NewItem);
	}
#endif
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

ItemBrowser::ItemBrowser(Vector3* Colors, Vector2 Position, Vector2 Scale) : EditorPanel(Colors, Position, Scale, Vector2(0.2, 1), Vector2(0.9, 4))
{
	CPPClasses = GetEditorUIClasses();
	TabBackground->SetHorizontal(false);
	TabBackground->Align = UIBox::E_REVERSE;
	ContentBox = new UIBox(false, 0);
	TabBackground->AddChild(ContentBox->SetPadding(0));

	UpdateLayout();
}


void ItemBrowser::UpdateLayout()
{
	Assets::ScanForAssets();
	ContentBox->DeleteChildren();

	BrowserScrollBox = new UIScrollBox(false, Scale - Vector2(0, 0.1), 25);
	BrowserScrollBox->SetMinSize(Vector2(0, 1.65));
	BrowserScrollBox->SetMaxSize(Vector2(MaxSize.X, 1.65));
	BrowserScrollBox->Align = UIBox::E_REVERSE;
	ContentBox->AddChild(BrowserScrollBox
		->SetPadding(0.01, 0, 0, 0));

	ContentBox->AddChild((new UIBackground(true, 0, UIColors[1], Vector2(Scale.X, 0.1)))
		->SetPadding(0)
		->AddChild((new UIButton(true, 0, 1, this, -2))
			->SetUseTexture(true, Editor::CurrentUI->Textures[8])
			->SetMinSize(Vector2(0.06))
			->SetSizeMode(UIBox::E_PIXEL_RELATIVE)
			->SetPadding(0.01))
		->AddChild((new UITextField(true, 0, UIColors[0], this, -3, Editor::CurrentUI->EngineUIText))
			->SetTextColor(UIColors[2])
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
			->SetMinSize(Vector2(Scale.X / Tabs.size(), 0.05f))
			->AddChild((new UIText(0.45, UIColors[2], i, Editor::CurrentUI->EngineUIText))
				->SetPadding(0.01, 0.01, 0, 0)));
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

	// if the bar isnt large enough to fit a single row of items, do nothing.
	if (ITEMS_PER_SLICE == 0)
	{
		return;
	}

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
#ifdef ENGINE_CSHARP
				if (i.Object.ID == CSharpObject::GetID() && i.Object.Name != "CSharpObject")
					DisplayedFiles.push_back(FileEntry(i.Name + ".cs", false));
				else
#endif
					DisplayedFiles.push_back(FileEntry(i.Name + ".cpp", false));
			}
		}
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

		auto ItemText = new UIText(0.35, UIColors[2], ItemName, Editor::CurrentUI->EngineUIText);
		ItemText->Wrap = true;
		ItemText->WrapDistance = 0.21;
		ItemText->SetTextWidthOverride(0.0);
		NewBackground->AddChild(ItemText
			->SetPadding(0.002));

		HorizontalSlices[i / ITEMS_PER_SLICE]->AddChild(NewBackground);
	}
}

void ItemBrowser::Tick()
{
	if (Input::IsRMBDown && !RMBDown && !Editor::DraggingTab && TabBackground->IsHovered() && !SelectedTab)
	{
		bool ButtonHovered = false;
		RMBDown = true;

		for (size_t i = 0; i < Buttons.size(); i++)
		{
			if (Buttons[i]->GetIsHovered())
			{
				SelectedButton = i;
				ButtonHovered = true;
				Editor::CurrentUI->ShowDropdownMenu(
				{
					EditorUI::DropdownItem("# " + FileUtil::GetFileNameWithoutExtensionFromPath(CurrentFiles[i].Name)),
					EditorUI::DropdownItem("Open", []()
					{
						Editor::CurrentUI->UIElements[3]->OnButtonClicked(SelectedButton);
					}),
					EditorUI::DropdownItem("Rename", []()
					{
						new RenameBox(CurrentFiles[SelectedButton].Name, 0);
					}),
					EditorUI::DropdownItem("Copy", []()
					{
						std::string NewFile = FileUtil::GetFilePathWithoutExtension(CurrentFiles[SelectedButton].Name) + "_Copy.";
						NewFile.append(FileUtil::GetExtension(CurrentFiles[SelectedButton].Name));
						if (!std::filesystem::exists(NewFile))
						{
							std::filesystem::copy(CurrentFiles[SelectedButton].Name, NewFile);
						}
						Editor::CurrentUI->UIElements[3]->UpdateLayout();
					}),
					EditorUI::DropdownItem("Delete", []()
					{
						std::filesystem::remove_all(CurrentFiles[SelectedButton].Name);
						Editor::CurrentUI->UIElements[3]->UpdateLayout();
					}),
				},
				Input::MouseLocation);
			}
		}

		if (!ButtonHovered)
		{
			Editor::CurrentUI->ShowDropdownMenu(
			{
				EditorUI::DropdownItem("# Create"),
				EditorUI::DropdownItem("Folder", []()
				{
					try
					{
						std::filesystem::create_directory(Editor::CurrentUI->CurrentPath + "/Folder");
					}
					catch(std::exception)
					{

					}
					Editor::CurrentUI->UIElements[3]->UpdateLayout();
				}),
				EditorUI::DropdownItem("Material", []()
				{
					EditorUI::CreateFile(Editor::CurrentUI->CurrentPath, "Material", "jsmat");
					Editor::CurrentUI->UIElements[3]->UpdateLayout();
				}),
				EditorUI::DropdownItem("Material Template", []()
				{
					EditorUI::CreateFile(Editor::CurrentUI->CurrentPath, "MaterialTemplate", "jsmtmp");
					Editor::CurrentUI->UIElements[3]->UpdateLayout();
				}),
				EditorUI::DropdownItem("Scene", []()
				{
					EditorUI::CreateFile(Editor::CurrentUI->CurrentPath, "Scene", "jscn");
					Editor::CurrentUI->UIElements[3]->UpdateLayout();
				}),
				EditorUI::DropdownItem("Particle", []()
				{
					EditorUI::CreateFile(Editor::CurrentUI->CurrentPath, "Particle", "jspart");
					Editor::CurrentUI->UIElements[3]->UpdateLayout();
				}),
				EditorUI::DropdownItem("Cubemap", []()
				{
					EditorUI::CreateFile(Editor::CurrentUI->CurrentPath, "Cubemap", "cbm");
					Editor::CurrentUI->UIElements[3]->UpdateLayout();
				})
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
				ChangedScene = true;
			}
			if (ext == "jscn" && !IsCPPClass)
			{
				EditorUI::OpenScene(CurrentFiles[DraggedButton].Name);
				return;
			}
			if (ext == "wav" && !IsCPPClass)
			{
				auto newObject = Objects::SpawnObject<SoundObject>(Transform(TargetSpawnLocation, 0, 1));
				newObject->LoadSound(FileUtil::GetFileNameWithoutExtensionFromPath(CurrentFiles[DraggedButton].Name));
				newObject->IsSelected = true;
				ChangedScene = true;
			}
			if (ext == "jspart" && !IsCPPClass)
			{
				auto newObject = Objects::SpawnObject<ParticleObject>(Transform(TargetSpawnLocation, 0, 1));
				newObject->LoadParticle(FileUtil::GetFileNameWithoutExtensionFromPath(CurrentFiles[DraggedButton].Name));
				newObject->IsSelected = true;
				ChangedScene = true;
			}

			if (IsCPPClass)
			{
				auto Item = GetContentsOfCurrentCPPFolder()[DraggedButton].Object;
				auto newObject = Objects::SpawnObjectFromID(Item.ID, Transform(TargetSpawnLocation, 0, 1));
				newObject->IsSelected = true;
				ChangedScene = true;
#ifdef ENGINE_CSHARP
				if (Item.ID == CSharpObject::GetID() && Item.Name != "CSharpObject")
				{
					dynamic_cast<CSharpObject*>(newObject)->LoadClass(Item.Name);
				}
#endif
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
			if (Editor::ModelFileExtensions.contains(FileUtil::GetExtension(file)))
			{
				ModelImporter::Import(file, Editor::CurrentUI->CurrentPath);
			}
			else
			{
				Importer::Import(file, Editor::CurrentUI->CurrentPath);
			}
			UpdateLayout();
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

	// If one of the item buttons was pressed
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
			EditorUI::OpenScene(CurrentFiles[Index].Name);
			return;
		}
		if (Ext == "png" || Ext == "jpg")
		{
			OS::OpenFile(CurrentFiles[Index].Name);
		}
		if (Ext == "wav")
		{
			Sound::PlaySound2D(Sound::LoadSound(FileUtil::GetFileNameWithoutExtensionFromPath(CurrentFiles[Index].Name)));
			Log::Print("Played sound " + FileUtil::GetFileNameFromPath(CurrentFiles[Index].Name));
		}
		if (Ext == "jsm")
		{
			Viewport::ViewportInstance->OpenTab(1, CurrentFiles[Index].Name);
		}
		if (Ext == "jsmat")
		{
			Viewport::ViewportInstance->OpenTab(2, CurrentFiles[Index].Name);
		}
		if (Ext == "jsmtmp")
		{
			Viewport::ViewportInstance->OpenTab(3, CurrentFiles[Index].Name);
		}
		if (Ext == "jspart")
		{
			Viewport::ViewportInstance->OpenTab(4, CurrentFiles[Index].Name);
		}
		if (Ext == "cbm")
		{
			Viewport::ViewportInstance->OpenTab(5, CurrentFiles[Index].Name);
		}
	}
	if (Index >= 0 && SelectedTab == 1)
	{
		if (GetContentsOfCurrentCPPFolder()[Index].IsFolder)
		{
			CPPPath.push_back(Index);
			UpdateLayout();
		}
#ifdef ENGINE_CSHARP
		else
		{
			std::string File = "Scripts/" + GetContentsOfCurrentCPPFolder()[Index].Name + ".cs";
			if (std::filesystem::exists(File))
			{
				OS::OpenFile(File);
			}
		}
#endif
	}
}
#endif