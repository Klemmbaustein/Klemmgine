#if EDITOR
#include "AssetBrowser.h"
#include <filesystem>
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Utility/FileUtility.h>
#include <Engine/Utility/StringUtility.h>
#include <Engine/Log.h>
#include <Engine/Importers/ModelConverter.h>
#include <Engine/Importers/Importer.h>
#include <Engine/OS.h>
#include <UI/EditorUI/Tabs/MaterialTab.h>
#include <Engine/Scene.h>
#include <UI/EditorUI/Viewport.h>
#include <UI/EditorUI/Tabs/MeshTab.h>
#include <UI/EditorUI/Tabs/ParticleEditorTab.h>

static std::string* CurrentPath = nullptr;

std::vector<AssetBrowser::BrowserItem> AssetBrowser::GetBrowserContents()
{
	std::vector<BrowserItem> Items;

	for (const auto& File : std::filesystem::directory_iterator(Path))
	{
		if (!File.is_directory())
		{
			continue;
		}
		BrowserItem Item;
		Item.Name = FileUtil::GetFileNameWithoutExtensionFromPath(File.path().u8string());
		std::string Ext = FileUtil::GetExtension(File.path().u8string());
		Item.Texture =  EditorUI::Textures[5];
		Item.Color = EditorUI::ItemColors["dir"];
		Item.Path = File.path().u8string();
		Items.push_back(Item);
	}

	for (const auto& File : std::filesystem::directory_iterator(Path))
	{
		if (File.is_directory())
		{
			continue;
		}
		BrowserItem Item;
		Item.Name = FileUtil::GetFileNameWithoutExtensionFromPath(File.path().u8string());
		std::string Ext = FileUtil::GetExtension(File.path().u8string());
		Item.Texture = EditorUI::Textures[EditorUI::ItemTextures[Ext]];
		Item.Color = EditorUI::ItemColors[Ext];
		Item.Path = File.path().u8string();
		Items.push_back(Item);
	}

	return Items;
}

void AssetBrowser::OnItemClicked(BrowserItem Item)
{
	if (std::filesystem::is_directory(Item.Path))
	{
#if _WIN32
		StrUtil::ReplaceChar(Item.Path, '\\', "/");
#endif
		Path = Item.Path + "/";
		CurrentPath = &Path;
		OnPathChanged();
		return;
	}
	std::string Extension = FileUtil::GetExtension(Item.Path);

	if (Extension == "jsmat")
	{
		auto NewTab = new MaterialTab(nullptr, Item.Path);
		if (Viewport::ViewportInstance->Parent->ChildrenAlign == ChildrenType::Tabs)
		{
			Viewport::ViewportInstance->Parent->AddTab(NewTab);
		}
		else
		{
			Viewport::ViewportInstance->AddTab(NewTab);
		}
		NewTab->Load(Item.Path);
	}

	if (Extension == "jsm")
	{
		auto NewTab = new MeshTab(nullptr, Item.Path);
		if (Viewport::ViewportInstance->Parent->ChildrenAlign == ChildrenType::Tabs)
		{
			Viewport::ViewportInstance->Parent->AddTab(NewTab);
		}
		else
		{
			Viewport::ViewportInstance->AddTab(NewTab);
		}
		NewTab->Load(Item.Path);
	}

	if (Extension == "jspart")
	{
		auto NewTab = new ParticleEditorTab(nullptr, Item.Path);
		if (Viewport::ViewportInstance->Parent->ChildrenAlign == ChildrenType::Tabs)
		{
			Viewport::ViewportInstance->Parent->AddTab(NewTab);
		}
		else
		{
			Viewport::ViewportInstance->AddTab(NewTab);
		}
	}

	if (Extension == "jscn")
	{
		EditorUI::OpenScene(Item.Path);
	}

	if (Extension == "png")
	{
		OS::OpenFile(Item.Path);
	}
}

void AssetBrowser::GoBack()
{
	if (Path[Path.size() - 1] == '/')
	{
		Path.pop_back();
	}
	Path = Path.substr(0, Path.find_last_of("/"));
	Path.append("/");
	CurrentPath = &Path;
}

void AssetBrowser::DeleteItem(BrowserItem Item)
{
	std::filesystem::remove_all(Item.Path);
}

void AssetBrowser::UpdateAll()
{
	for (UICanvas* c : Graphics::UIToRender)
	{
		if (dynamic_cast<AssetBrowser*>(c))
		{
			static_cast<AssetBrowser*>(c)->OnPathChanged();
		}
	}

}

AssetBrowser::AssetBrowser(EditorPanel* Parent) : ItemBrowser(Parent, "Assets")
{
	DefaultDropdown =
	{
		EditorUI::DropdownItem("Import file", []()
			{
			std::string file = OS::ShowOpenFileDialog();
			if (std::filesystem::exists(file))
			{
				if (EditorUI::ModelFileExtensions.contains(FileUtil::GetExtension(file)))
				{
					ModelImporter::Import(file, *CurrentPath);
				}
				else
				{
					Importer::Import(file, *CurrentPath);
				}
			}
			UpdateAll();
			}, true),

		EditorUI::DropdownItem("New folder", []() 
			{ 
				std::filesystem::create_directories(*CurrentPath + "/Folder");
				UpdateAll();
			}),
		EditorUI::DropdownItem("New material", []()
			{
				EditorUI::CreateFile(*CurrentPath, "Material", "jsmat");
				UpdateAll();
			}),
		EditorUI::DropdownItem("New scene", []()
			{
				EditorUI::CreateFile(*CurrentPath, "Scene", "jscn");
				UpdateAll();
			}),
				EditorUI::DropdownItem("New particle", []()
			{
				EditorUI::CreateFile(*CurrentPath, "Particle", "jspart");
				UpdateAll();
			}, true),
		EditorUI::DropdownItem("Open in file explorer", []()
			{
#if _WIN32
				std::string Path = *CurrentPath;
				StrUtil::ReplaceChar(Path, '/', "\\");
				system(("explorer.exe \"" + Path + "\"").c_str());
#else
				system(("xdg-open \"" + *CurrentPath + "\"").c_str());
#endif
			}),
	};

	EmptyText = "Right-click to create new assets";
	Path = "Content/";
	CurrentPath = &Path;
	OnPathChanged();
}
#endif