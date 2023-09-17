#ifdef EDITOR
#pragma once
#include "UI/Default/UICanvas.h"
#include "UI/Default/TextRenderer.h"
#include <UI/UIBackground.h>
#include "UI/UIButton.h"
#include <UI/UITextField.h>
#include <UI/UIText.h>
#include <UI/EditorUI/EditorPanel.h>
#include "Tabs/MeshTab.h"
#include "Tabs/ParticleEditorTab.h"
#include <Engine/TypeEnun.h>
#include <Objects/WorldObject.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Engine/Build/Build.h>

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
	extern bool DraggingPopup;
	extern bool HoveringPopup;
	extern bool PrevHoveringPopup;
	extern Vector2 DragMinMax;
	extern Vector2 NewDragMinMax;
	extern bool IsBakingScene;
	inline std::map<std::string, Vector3> ItemColors
	{
		std::pair("dir", Vector3(0.8f, 0.5f, 0)),
		std::pair("jsmat", Vector3(0, 0.7f, 0)),
		std::pair("jsm", Vector3(0, 0, 0.75f)),
		std::pair("jscn", Vector3(1.0f, 0.4f, 0.4f)),
		std::pair("png", Vector3(0.3f, 0, 1)),
		std::pair("cbm", Vector3(0.7f, 0.1f, 0.4f)),
		std::pair("jspart", Vector3(0.7f, 0.4f, 0.4f)),
		std::pair("wav", Vector3(0.7f, 0, 0.4f)),
		std::pair("cpp", Vector3(0.5f)),
		std::pair("setting", Vector3(1)),
		std::pair("bkdat", Vector3(1.0f, 0.6f, 0.2f)),
		std::pair("cs", Vector3(0.603921569f, 0.28627451f, 0.576470588f))
	};
	inline std::map<std::string, unsigned int> ItemTextures
	{
		std::pair("dir", 5),
		std::pair("jsmat", 9),
		std::pair("jsm", 11),
		std::pair("jscn", 7),
		std::pair("png", 18),
		std::pair("cbm", 17),
		std::pair("wav", 6),
		std::pair("cpp", 0),
		std::pair("cs", 22),
		std::pair("jspart", 19),
		std::pair("bkdat", 27)
	};

	inline std::set<std::string> ModelFileExtensions =
	{
		"obj",
		"fbx",
		"gltf",
		"glb",
	};
}

class EditorUI : public UICanvas
{
public:
#if ENGINE_CSHARP
	static FILE* DebugProcess;
	static void ReadProcessOutput();
	static std::thread* DebugProcessIOThread;
	static void LaunchInEditor();
	static void RebuildAndHotReload();
	static std::string LaunchInEditorArgs;
	static void SetLaunchCurrentScene(bool NewLaunch);
#endif
	static void SaveCurrentScene();
	static void OpenScene(std::string NewScene);
	static bool GetUseLightMode();
	static void SetUseLightMode(bool NewLightMode);

	static void CreateFile(std::string Path, std::string Name, std::string Ext);
	EditorUI();
	void OnLeave(void(*ReturnF)());
	virtual void Tick() override;
	void GenUITextures();

	static bool IsTitleBarHovered();

	UIBox* DraggedItem = nullptr;
	UIBox* Dropdown = nullptr;

	static inline constexpr uint32_t NumUIColors = 4;

	Vector3 UIColors[NumUIColors] =
	{
		Vector3(0.125f, 0.125f, 0.13f),	//Default background
		Vector3(0.08f),					//Dark background
		Vector3(1),						//Highlight color
		Vector3(0.2f),					//Brighter background
	};
	TextRenderer* EngineUIText = new TextRenderer("Font.ttf", 90);

	enum class CursorType
	{
		Default = 0,
		Grab = 1,
		Cross = 2,
		Loading = 3,
		Resize_WE = 4,
		Resize_NS = 5,
		Resize_All = 6,
		TextHover = 7,
		End
	};

	struct DropdownItem
	{
		std::string Title;
		void (*OnPressed)() = nullptr;
	};

	void ShowDropdownMenu(std::vector<DropdownItem> Menu, Vector2 Position);

	EditorPanel* UIElements[7] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

	CursorType CurrentCursor = CursorType::Default;
	std::vector<unsigned int> Textures;
	std::string CurrentPath = "Content";

protected:
	SDL_Cursor* Cursors[(int)CursorType::End];
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

	void BakeScene();

	void OnResized();
protected:
	friend WorldObject;
};
#endif