#ifdef EDITOR
#pragma once
#include "UI/Default/UICanvas.h"
#include "UI/Default/TextRenderer.h"
#include <UI/UIBackground.h>
#include "UI/UIButton.h"
#include <UI/UITextField.h>
#include <UI/UIText.h>
#include <UI/EditorUI/EditorTab.h>
#include <UI/EditorUI/MaterialTemplateTab.h>
#include "MeshTab.h"
#include "ParticleEditorTab.h"
#include <Engine/TypeEnun.h>
#include <Objects/WorldObject.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Engine/Importers/Build/Build.h>

#include <SDL.h>
#include <thread>

class EditorUI;
class UIVectorField;
class UITextField;
extern bool ChangedScene;

namespace Editor
{
	extern EditorUI* CurrentUI;
	extern bool DraggingTab;
	extern bool TabDragHorizontal;
	extern Vector2 DragMinMax;
	extern Vector2 NewDragMinMax;
}

class EditorUI : public UICanvas
{
public:
	EditorUI();
	void OnLeave(void(*ReturnF)());
	virtual void Tick() override;
	void GenUITextures();

	UIBox* DraggedItem = nullptr;

	Vector3 UIColors[3] =
	{
		Vector3(0.1, 0.1, 0.11),	//Default background
		Vector3(0.07f),				//Dark background
		Vector3(1)					//Highlight color
		//Vector3(0.9, 0.9, 0.9),	//Default background
		//Vector3(0.5f),			//Dark background
		//Vector3(0.1)				//Highlight color};
	};
	TextRenderer* EngineUIText = new TextRenderer("Font.ttf", 90);

	enum CursorType
	{
		E_DEFAULT = 0,
		E_GRAB = 1,
		E_CROSS = 2,
		E_LOADING = 3,
		E_RESIZE_WE = 4,
		E_RESIZE_NS = 5,
		E_LAST_CURSOR = 6
	};

	EditorTab* UIElements[5];

	CursorType CurrentCursor = E_DEFAULT;
	std::vector<unsigned int> Textures;
	std::string CurrentPath = "Content";

protected:
	SDL_Cursor* Cursors[6];

	std::set<std::string> CollapsedItems;
	std::vector<std::string> ObjectCategories;


	std::string ToShortString(float val);
	struct ObjectListItem
	{
		ObjectListItem(std::string CategoryName, std::vector<ObjectListItem> Children, bool IsScene, bool IsCollapsed)
		{
			this->Name = CategoryName;
			this->Children = Children;
			this->IsScene = IsScene;
			this->IsCollapsed = IsCollapsed;
		}
		ObjectListItem(WorldObject* Object, int ListIndex)
		{
			this->Object = Object;
			if (Object) this->Name = Object->GetName();
			this->ListIndex = ListIndex;
		}

		bool IsCollapsed = false;
		std::vector<ObjectListItem> Children;
		std::string Name;
		WorldObject* Object = nullptr;
		int ListIndex = -1;
		bool IsScene = false;
	};
	std::vector<ObjectListItem> GetObjectList();



	UIBox* DropDownMenu = nullptr;
	Vector2 DropDownMenuPosition;
	friend WorldObject;
};
#endif