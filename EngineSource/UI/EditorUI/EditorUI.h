#ifdef EDITOR
#pragma once
#include "UI/Default/UICanvas.h"
#include "UI/Default/TextRenderer.h"
#include <UI/UIBackground.h>
#include "UI/UIButton.h"
#include <UI/UITextField.h>
#include <UI/UIText.h>
#include <UI/EditorUI/EditorPanel.h>
#include <UI/EditorUI/Tabs/MaterialTemplateTab.h>
#include "Tabs/MeshTab.h"
#include "Tabs/ParticleEditorTab.h"
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

	inline std::map<std::string, Vector3> ItemColors
	{
		std::pair("dir", Vector3(0.8, 0.5, 0)),
		std::pair("jsmat", Vector3(0, 0.7, 0)),
		std::pair("jsmtmp", Vector3(0.1, 0.4, 0)),
		std::pair("jsm", Vector3(0, 0, 0.75)),
		std::pair("jscn", Vector3(1.0, 0.4, 0.4)),
		std::pair("png", Vector3(0.3, 0, 1)),
		std::pair("cbm", Vector3(0.7, 0.1, 0.4)),
		std::pair("jspart", Vector3(0.7, 0.4, 0.4)),
		std::pair("wav", Vector3(0.7, 0, 0.4)),
		std::pair("cpp", Vector3(0.5))
	};
	inline std::map<std::string, unsigned int> ItemTextures
	{
		std::pair("dir", 5),
		std::pair("jsmat", 9),
		std::pair("jsmtmp", 10),
		std::pair("jsm", 11),
		std::pair("jscn", 7),
		std::pair("png", 18),
		std::pair("cbm", 17),
		std::pair("wav", 6),
		std::pair("cpp", 0)
	};
}

class EditorUI : public UICanvas
{
public:
	EditorUI();
	void OnLeave(void(*ReturnF)());
	virtual void Tick() override;
	void GenUITextures();

	UIBox* DraggedItem = nullptr;
	UIBox* Dropdown = nullptr;
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

	struct DropdownItem
	{
		std::string Title;
		void (*OnPressed)() = nullptr;
	};

	void ShowDropdownMenu(std::vector<DropdownItem> Menu, Vector2 Position);

	EditorPanel* UIElements[7] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

	CursorType CurrentCursor = E_DEFAULT;
	std::vector<unsigned int> Textures;
	std::string CurrentPath = "Content";

protected:
	SDL_Cursor* Cursors[6];
	std::vector<DropdownItem> CurrentDropdown;

public:
	std::set<std::string> CollapsedItems;
	std::vector<std::string> ObjectCategories;


	static std::string ToShortString(float val);

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
	void OnButtonClicked(int Index);
protected:
	friend WorldObject;
};
#endif