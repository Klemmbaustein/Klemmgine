#if EDITOR
#include "ClassesBrowser.h"
#include <Engine/Subsystem/CSharpInterop.h>
#include <Objects/CSharpObject.h>
#include <UI/EditorUI/Popups/ClassCreator.h>
#include <UI/EditorUI/EditorUI.h>
#include <Engine/OS.h>

std::vector<ClassesBrowser::EditorClassesItem> ClassesBrowser::CPPClasses;
std::vector<size_t> ClassesBrowser::CPPPath;

std::vector<ClassesBrowser::EditorClassesItem> ClassesBrowser::GetEditorUIClasses()
{
	std::vector<std::string> IDs;
	EditorClassesItem RootPath;

	std::vector<ObjectDescription> AllObjects = Objects::ObjectTypes;

#ifdef ENGINE_CSHARP
	for (auto& i : CSharpInterop::CSharpSystem->GetAllClasses())
	{
		ObjectDescription Descr = ObjectDescription(i, CSharpObject::GetID());
		AllObjects.push_back(Descr);
	}
#endif


	for (const auto& Object : AllObjects)
	{
		// First seperate the Category into multiple names. For example: "Default/Rendering" -> { "Default", "Rendering" }
		std::string CurrentPath = Objects::GetCategoryFromID(Object.ID);
		if (Object.ID == CSharpObject::GetID() && Object.Name != "CSharpObject")
		{
			size_t Index = Object.Name.find_last_of("/");
			CurrentPath = Object.Name.substr(0, Index);
			if (Index == std::string::npos)
			{
				CurrentPath.clear();
			}
		}
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
	return RootPath.SubItems;
}
std::string ClassesBrowser::GetCurrentCPPPathString()
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


void ClassesBrowser::UpdateClasses()
{
	CPPClasses = GetEditorUIClasses();

	std::vector<EditorClassesItem> CurrentItems = CPPClasses;
	for (size_t i : CPPPath)
	{
		if (CurrentItems.size() <= i)
		{
			CPPPath.clear();
			break;
		}
		CurrentItems = CurrentItems[i].SubItems;
	}
	OnPathChanged();
}

std::vector<ClassesBrowser::EditorClassesItem> ClassesBrowser::GetContentsOfCurrentCPPFolder()
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

ClassesBrowser::ClassesBrowser(EditorPanel* Parent) : ItemBrowser(Parent, "Classes")
{
	DefaultDropdown =
	{
		EditorUI::DropdownItem("New C# class", []()
			{
				new ClassCreator();
			}),
		EditorUI::DropdownItem("Open Solution", []()
			{
				OS::OpenFile(Build::GetProjectBuildName() + ".sln");
			}, true)

	};
	Path = GetCurrentCPPPathString();
	UpdateClasses();
	OnPathChanged();
}

std::vector<ClassesBrowser::BrowserItem> ClassesBrowser::GetBrowserContents()
{
	std::vector<BrowserItem> Items;
	auto CItems = GetContentsOfCurrentCPPFolder();
	for (auto& i : CItems)
	{
		BrowserItem New;
		New.Name = FileUtil::GetFileNameFromPath(i.Name);
		std::string Ext = (i.Object.ID == CSharpObject::GetID() && i.Object.Name != "CSharpObject") ? "cs" : "cpp";

		New.Color = i.IsFolder ? EditorUI::ItemColors["dir"] : EditorUI::ItemColors[Ext];
		New.Texture = i.IsFolder ? EditorUI::Textures[5] : EditorUI::Textures[EditorUI::ItemTextures[Ext]];
		New.Renameable = false;
		New.Openable = i.IsFolder;
		New.TypeID = i.Object.ID;
		New.Path = New.Name;
		New.Deleteable = false;
		Items.push_back(New);
	}
	return Items;
}

void ClassesBrowser::OnItemClicked(BrowserItem Item)
{
	auto Content = GetContentsOfCurrentCPPFolder();
	for (size_t i = 0; i < Content.size(); i++)
	{
		if (Item.Name == Content[i].Name && Content[i].IsFolder)
		{
			CPPPath.push_back(i);
			break;
		}
	}
	Path = GetCurrentCPPPathString();
	OnPathChanged();

}

void ClassesBrowser::GoBack()
{
	if (!CPPPath.empty())
	{
		CPPPath.pop_back();
	}
	Path = GetCurrentCPPPathString();
}
#endif