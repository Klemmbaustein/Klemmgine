#ifdef EDITOR
#include "EditorUI.h"
#include "Utility/stb_image.h"
#include "Engine/FileUtility.h"
#include <filesystem>
#include "Math/Math.h"
#include "Math/Collision/Collision.h"
#include "Engine/Scene.h"
#include <Engine/EngineProperties.h>
#include <UI/UIScrollBox.h>	
#include <UI/EditorUI/UIVectorField.h>
#include <UI/EditorUI/LogUI.h>
#include <UI/EditorUI/Toolbar.h>
#include <UI/EditorUI/ItemBrowser.h>
#include <UI/EditorUI/StatusBar.h>
#include <UI/EditorUI/Viewport.h>
#include <GL/glew.h>
#include <Engine/Log.h>
#include <Engine/Input.h>

namespace Editor
{
	EditorUI* CurrentUI = nullptr;
	bool DraggingTab = false;
	bool TabDragHorizontal = false;
	Vector2 DragMinMax;
	Vector2 NewDragMinMax = DragMinMax;
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

// Experimental
#define UI_LIGHT_MODE 0

EditorUI::EditorUI()
{
	if (UI_LIGHT_MODE)
	{
		size_t it = 0;
		for (auto i : {
			Vector3(0.7, 0.7, 0.725),	//Default background
			Vector3(0.5f),			//Dark background
			Vector3(0)			//Highlight color};
			})
		{
			UIColors[it++] = i;
		}
	}

	GenUITextures();

	Cursors[0] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	Cursors[1] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	Cursors[2] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	Cursors[3] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
	Cursors[4] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	Cursors[5] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	Editor::CurrentUI = this;
	//new UIBackground(true, -1, 0.5, 2);

	UIElements[0] = new StatusBar(UIColors);
	UIElements[1] = new LogUI(UIColors, Vector2(-0.7, -1), Vector2(1.25, 0.4));
	UIElements[2] = new Toolbar(UIColors, Vector2(-0.7, 0.73), Vector2(1.25, 0.22));
	UIElements[3] = new ItemBrowser(UIColors, Vector2(-1, -1), Vector2(0.3, 1.95));
	UIElements[4] = new Viewport(UIColors, Vector2(-0.7, -0.6), Vector2(1.25, 1.33));

}

void EditorUI::OnLeave(void(*ReturnF)())
{
	ReturnF();
}

void EditorUI::Tick()
{
	if (!Editor::DraggingTab)
	{
		Editor::NewDragMinMax = Vector2(-1, 1);
	}
	Editor::DragMinMax = Editor::NewDragMinMax;

	if (UI::HoveredButton)
	{
		CurrentCursor = E_GRAB;
	}
	if (CurrentCursor < E_LAST_CURSOR && CurrentCursor >= 0)
	{
		SDL_SetCursor(Cursors[CurrentCursor]);
	}
	CurrentCursor = Editor::DraggingTab ? (Editor::TabDragHorizontal ? E_RESIZE_WE : E_RESIZE_NS) : E_DEFAULT;


	if (DraggedItem)
	{
		DraggedItem->SetPosition(Input::MouseLocation - DraggedItem->GetUsedSize() / 2);
		if (!Input::IsLMBDown)
		{
			delete DraggedItem;
			DraggedItem = nullptr;
		}
	}
}



std::string EditorUI::ToShortString(float val)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << val;
	return stream.str();
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



#endif