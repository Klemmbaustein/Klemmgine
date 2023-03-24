#ifdef EDITOR
// MSVC does not like the strerror() at line 703, without this it will throw a compiler error
#define _CRT_SECURE_NO_WARNINGS
#include "EditorUI.h"
#include "Utility/stb_image.h"
#include "Engine/FileUtility.h"
#include <filesystem>
#include <algorithm>
#include "Math/Math.h"
#include <fstream>
#include "Math/Collision/Collision.h"
#include "Engine/Importers/ModelConverter.h"
#include "Engine/Importers/Importer.h"
#include "Objects/MeshObject.h"
#include "Engine/Scene.h"
#include <Engine/Input.h>
#include <Engine/Console.h>
#include <Engine/Log.h>
#include <Engine/OS.h>
#include <Engine/EngineProperties.h>
#include <thread>
#include <Engine/Importers/Build/Build.h>
#include <World/Assets.h>
#include <World/Graphics.h>
#include <World/Stats.h>
#include <UI/UIScrollBox.h>	
#include <Rendering/Texture/Texture.h>
#include <UI/EditorUI/UIVectorField.h>
#include <UI/EditorUI/LogUI.h>
#include <UI/EditorUI/Toolbar.h>
#include <UI/EditorUI/ItemBrowser.h>
#include <Objects/ParticleObject.h>
#include <Objects/InstancedMeshObject.h>
#include <Objects/SoundObject.h>
#include <Rendering/Mesh/InstancedModel.h>
#include <GL/glew.h>

namespace Editor
{
	EditorUI* CurrentUI = nullptr;
	bool DraggingTab = false;
	bool TabDragHorizontal = false;
	Vector2 DragMinMax;
}
// Collision model for the arrows

Collision::Box ArrowBoxX
(
	 0.0f, 1.0f,
	-0.1f, 0.1f,
	-0.1f, 0.1f

);

Collision::Box ArrowBoxY
(
	-0.1f, 0.1f,
	 0.0f, 1.0f,
	-0.1f, 0.1f

);

Collision::Box ArrowBoxZ
(
	-0.1f, 0.1f,
	-0.1f, 0.1f,
	-1.0f, 0.0f
	
);



EditorUI::EditorUI()
{
	GenUITextures();

	Cursors[0] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	Cursors[1] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	Cursors[2] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	Cursors[3] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
	Cursors[4] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	Cursors[5] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	Editor::CurrentUI = this;
	//new UIBackground(true, -1, 0.5, 2);

	new LogUI(UIColors, Vector2(-0.7, -1), Vector2(1.25, 0.4));
	new Toolbar(UIColors, Vector2(-0.7, 0.75), Vector2(1.25, 0.25));
	new ItemBrowser(UIColors, Vector2(-1, -1), Vector2(0.3, 2));

}

void EditorUI::OnLeave(void(*ReturnF)())
{
	ReturnF();
}

void EditorUI::Tick()
{
	if (!Editor::DraggingTab)
	{
		Editor::DragMinMax = Vector2(-1, 1);
	}
	if (UI::HoveredButton)
	{
		CurrentCursor = E_GRAB;
	}
	if (CurrentCursor < E_LAST_CURSOR && CurrentCursor >= 0)
	{
		SDL_SetCursor(Cursors[CurrentCursor]);
	}
	CurrentCursor = Editor::DraggingTab ? E_RESIZE_WE : E_DEFAULT;
}

std::vector<EditorClassesItem> EditorUI::GetContentsOfCurrentCPPFolder()
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

std::string EditorUI::ToShortString(float val)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << val;
	return stream.str();
}

std::string EditorUI::GetCurrentCPPPathString()
{
	std::string PathString = "C++";
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

void EditorUI::GenUITextures()
{
	const int ImageSize = 20;
	std::string Images[ImageSize]
	{												//Texture Indices
		"EditorContent/Images/CPPClass.png",		//00 -> C++ class icon
		"EditorContent/Images/Wireframe.png",		//01 -> Symbol for button to toggle wireframe
		"EditorContent/Images/Save.png",			//02 -> Save Button
		"EditorContent/Images/Build.png",			//03 -> Package button
		"EditorContent/Images/X.png",				//04 -> X Symbol
		"EditorContent/Images/Folder.png",			//05 -> Folder symbol for item browser
		"EditorContent/Images/Sound.png",			//06 -> Sound symbol for item browser
		"EditorContent/Images/Scene.png",			//07 -> Scene symbol for item browser
		"EditorContent/Images/ExitFolder.png",		//08 -> Icon used to navigate back one folder
		"EditorContent/Images/Material.png",		//09 -> Material symbol for item browser
		"EditorContent/Images/MaterialTemplate.png",//10 -> Material Template symbol for item browser
		"EditorContent/Images/Model.png",			//11 -> Model symbol for item browser
		"EditorContent/Images/Reload.png",			//12 -> Reload symbol
		"EditorContent/Images/ExpandedArrow.png",	//13 -> Expanded arrow
		"EditorContent/Images/CollapsedArrow.png",	//14 -> Collapsed arrow
		"EditorContent/Images/Preferences.png",		//15 -> Collapsed arrow
		"EditorContent/Images/Checkbox.png",		//16 -> Checked checkbox
		"EditorContent/Images/Cubemap.png",			//17 -> Cubemap icon
		"EditorContent/Images/Texture.png",			//18 -> Texture icon
	};

	for (int i = 0; i < Textures.size(); i++)
	{
		glDeleteTextures(1, &Textures.at(i));
	}
	for (int i = 0; i < ImageSize; i++)
	{
		int TextureWidth = 0;
		int TextureHeigth = 0;
		int BitsPerPixel = 0;
		stbi_set_flip_vertically_on_load(true);
		auto TextureBuffer = stbi_load(Images[i].c_str(), &TextureWidth, &TextureHeigth, &BitsPerPixel, 4);


		GLuint TextureID;
		glGenTextures(1, &TextureID);
		glBindTexture(GL_TEXTURE_2D, TextureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TextureWidth, TextureHeigth, 0, GL_RGBA, GL_UNSIGNED_BYTE, TextureBuffer);

		Textures.push_back(TextureID);
		if (TextureBuffer)
		{
			stbi_image_free(TextureBuffer);
		}
	}
}

std::vector<EditorUI::ObjectListItem> EditorUI::GetObjectList()
{
	std::vector<ObjectListItem> ObjectList;
	ObjectCategories.clear();
	size_t ListIndex = 0;
	for (WorldObject* o : Objects::AllObjects)
	{
		ObjectListItem* SceneList = nullptr;
		// Get the list for the scene the object belongs to
		for (auto& item : ObjectList)
		{
			if (item.Name == FileUtil::GetFileNameFromPath(o->CurrentScene))
			{
				SceneList = &item;
			}
		}

		if (!SceneList)
		{
			std::string SceneName = FileUtil::GetFileNameFromPath(o->CurrentScene);
			ObjectList.push_back(ObjectListItem(SceneName, {}, true, CollapsedItems.contains("OBJ_CAT_" + SceneName)));
			ObjectCategories.push_back(FileUtil::GetFileNameFromPath(o->CurrentScene));
			ObjectList[ObjectList.size() - 1].ListIndex = ObjectCategories.size() - 1;
			SceneList = &ObjectList[ObjectList.size() - 1];
		}

		// Seperate the Object's category into multiple strings
		std::string CurrentPath = Objects::GetCategoryFromID(o->GetObjectDescription().ID);
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

		ObjectListItem* CurrentList = SceneList;
		if (SceneList->IsCollapsed) continue;
		for (const auto& elem : PathElements)
		{
			ObjectListItem* NewList = nullptr;
			for (auto& c : CurrentList->Children)
			{
				if (c.Name != elem) continue;
				NewList = &c;
				break;
			}

			if (!NewList && !CurrentList->IsCollapsed)
			{
				int it = 0;
				while (true)
				{
					if (it >= (int)CurrentList->Children.size() || CurrentList->Children[it].Object)
					{
						break;
					}
					it++;
				}
				ObjectCategories.push_back(elem);
				CurrentList->Children.insert(CurrentList->Children.begin() + it, ObjectListItem(elem, {}, false, CollapsedItems.contains("OBJ_CAT_" + elem)));
				CurrentList->Children[it].ListIndex = ObjectCategories.size() - 1;
				NewList = &CurrentList->Children[it];
			}
			CurrentList = NewList;
		}
		if (CurrentList && !CurrentList->IsCollapsed)
		{
			CurrentList->Children.push_back(ObjectListItem(o, ListIndex));
		}
		ListIndex++;
	}
	return ObjectList;
}

std::vector<EditorClassesItem> EditorUI::GetEditorUIClasses()
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

#endif