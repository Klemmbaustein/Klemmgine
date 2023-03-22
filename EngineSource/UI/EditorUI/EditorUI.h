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

#include <SDL.h>
#include <thread>

class EditorUI;
class UIVectorField;
class UITextField;
extern bool ChangedScene;

namespace ContentBrowser
{

	struct Asset
	{
		std::string FilePath;
		bool IsDirectory = false;
	};

	extern std::string CurrentFilePath;

	extern std::vector<Asset> ContentAssets;
	void UpdateContentAssets();
}

namespace EngineUIVariables
{
	struct Tab
	{
		Tab(int Type, std::string Name, std::string Path, bool CanBeClosed = true)
		{
			this->Type = Type;
			this->Name = Name;
			this->CanBeClosed = CanBeClosed;
			this->Path = Path;

		}
		int Type;
		std::string Name;
		bool CanBeClosed;
		std::string Path;
	};
	extern std::vector<Tab> Tabs;
}

struct PopUpButton
{
	PopUpButton(std::string Text, bool CallFunc, void(*FPointer)())
	{
		this->Text = Text;
		this->CallFunc = CallFunc;
		this->FPointer = FPointer;
	}
	std::string Text = "PopUp";
	bool CallFunc = true;
	void(*FPointer)() = nullptr;
};

struct EditorDropdownItem
{
	EditorDropdownItem(std::string Name, int Index, Vector3 Color)
	{
		this->Index = Index;
		this->Name = Name;
		this->Color = Color;
	}
	Vector3 Color;
	int Index = 0;
	std::string Name;
};

struct EditorClassesItem
{
	std::string Name;
	ObjectDescription Object = ObjectDescription("", 0);
	std::vector<EditorClassesItem> SubItems;
	bool IsFolder = false;
};

namespace Editor
{
	extern EditorUI* CurrentUI;
}

class EditorUI : public UICanvas
{
public:
	EditorUI();
	void OnLeave(void(*ReturnF)());
	void ShowPopUpWindow(std::string Message, std::vector<PopUpButton> Buttons);
	virtual void OnButtonClicked(int Index) override;
	virtual void Tick() override;
	virtual void OnButtonDragged(int Index) override;
	void GenUITextures();
	void UpdateObjectList();

	UIBackground* DragUI = nullptr;
	Model* ArrowModel = nullptr;
	FramebufferObject* ArrowFramebuffer = nullptr;
	FramebufferObject* OutlineFramebuffer = nullptr;

	Vector3 UIColors[3] =
	{
		Vector3(0.1, 0.1, 0.11),	//Default background
		Vector3(0.07f),				//Dark background
		Vector3(1)					//Highlight color
		//Vector3(0.9, 0.9, 1),	//Default background
		//Vector3(0.6f),				//Dark background
		//Vector3(1, 0, 1)					//Highlight color};
	};
	void UpdateLogMessages();
protected:
	float FPSUpdateTimer = 0;
	unsigned int DisplayedFPS = 60;
	std::map<std::string, Vector3> IconColors =
	{
		std::pair("dir", Vector3(0.8, 0.5, 0)),
		std::pair("jsm", Vector3(0, 0, 0.75)),
		std::pair("jsmat", Vector3(0, 0.7, 0)),
		std::pair("jsmtmp", Vector3(0.1, 0.4, 0)),
		std::pair("png", Vector3(0.3, 0, 1)),
		std::pair("jscn", Vector3(1.0, 0.4, 0.4)),
		std::pair("cpp", Vector3(0.5)),
		std::pair("wav", Vector3(0.7, 0.1, 0.4)),
		std::pair("jspart", Vector3(0.7, 0.4, 0.4)),
		std::pair("cbm", Vector3(0.7, 0, 0.4))
	};

	std::map<std::string, Vector3> ObjectColors =
	{
		std::pair("MeshObject", Vector3(0, 0, 0.75)),
		std::pair("InstancedMeshObject", Vector3(0, 0.5, 0.3)),
		std::pair("None", Vector3(0.5)),
		std::pair("SoundObject", Vector3(0.7, 0.1, 0.4))
	};

	std::vector<std::string> ObjectCategories;

	std::map<int, Vector3> TabColors =
	{
		std::pair(0, Vector3(1.0, 0.4, 0.4)),	//Viewport
		std::pair(1, Vector3(0, 0, 0.75)),		//Model editor
		std::pair(2, Vector3(0.1, 0.4, 0)),		//Material template editor
		std::pair(3, Vector3(0, 0.7, 0)),		//Material editor
		std::pair(4, Vector3(0.7, 0.4, 0.4)),	//Particle editor
		std::pair(5, Vector3(0.5)),				//Preferences
		std::pair(6, Vector3(0.7, 0, 0.4))		//Cubemap editor
	};

	WorldObject* PreviousSelectedObject = nullptr;
	std::vector<UIBox*> Properties;
	std::vector<UIButton*> ItemBrowserButtons;
	std::vector<UIText*> ItemBrowserTexts;
	UIText* TopBarText[3];
	UIBox* LeftBoxes[3];
	UIBox* RightBoxes[2];
	UIBox* TabBox;
	unsigned int RenamedIndex = 0;
	UIScrollBox* LogScrollObject = nullptr;
	UITextField* ItemBrowserPath;
	UITextField* LogConsolePromt = nullptr;
	std::vector<PopUpButton> PopUpButtons;
	std::string PopUpMesage;
	std::thread* BuildThread = nullptr;
	TextRenderer EngineUIText = TextRenderer("Font.ttf", 90);

	std::set<std::string> CollapsedItems;

	std::vector<unsigned int> Textures;
	std::vector<EditorTab*> TabItems;
	UIBackground* PopUpMenu = nullptr;
	bool LMBDown = false;
	bool RMouseLock = false;
	bool RenamingContentAsset = false;
	bool RenamingWorldObject = false;
	bool ContentBrowserMode = false;
	int DraggedButton = 0;
	int ShouldStopDragging = 0;
	SDL_Cursor* CrossCursor;
	SDL_Cursor* GrabCursor;
	SDL_Cursor* LoadCursor;
	SDL_Cursor* DefaultCursor;
	void UpdateItemBrowser();
	void UpdateContextMenu();
	void UpdateTabs();
	void OpenTab(int TabID, std::string File);
	void GenerateSceneSettings();
	UIButton* MakeIconButton(UIBox* Parent, unsigned int Texture, std::string Name, unsigned int ButtonIndex);
	std::string ToShortString(float val);
	std::vector<EditorClassesItem> GetEditorUIClasses();
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
	void GenerateObjectListSection(std::vector<ObjectListItem> Section, float Depth);

	struct ContextMenuSection
	{
		void* Variable;
		Type::TypeEnum Type;
		std::string Name;
		bool Normalized;
		ContextMenuSection(void* Variable, Type::TypeEnum Type, std::string Name, bool Normalized = false)
		{
			this->Variable = Variable;
			this->Type = Type;
			this->Name = Name;
			this->Normalized = Normalized;
		}
	};
	UITextField* GenerateNewContextMenuTextField(std::string Content);
	std::vector<EditorClassesItem> GetContentsOfCurrentCPPFolder();
	std::string GetCurrentCPPPathString();

	std::vector<std::string> SceneCategories;
	std::vector<ContextMenuSection> SceneSettings;
	std::vector<UIBox*> SceneButtons;
	void ContextMenu_GenerateSection(std::vector<ContextMenuSection> Section, std::string Name, WorldObject* ContextObject, unsigned int Index);

	UIBox* DropDownMenu = nullptr;
	Vector2 DropDownMenuPosition;
	std::vector<EditorDropdownItem> DropDownItems;
	void MakeDropDownMenu(Vector2 Position, std::vector<EditorDropdownItem> DropDownItems);
	bool RMBDown = false;
	friend WorldObject;
	std::vector<EditorClassesItem> CPPClasses;
	std::vector<size_t> CPPPath;
};
#endif