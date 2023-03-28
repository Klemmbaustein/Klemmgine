#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>

class WorldObject;
class Model;
class EditorTab;

class Viewport : public EditorPanel
{
	bool ViewportLock = false;
	bool PressedLMB = false;
	Vector2 InitialMousePosition = 0;
	UIBox* TabBox = nullptr;
	bool Dragging = false;
	Vector3 Axis;
	Vector3 BoxAxis = Vector3(1, 1, 0);
	bool FirstDragFrame = 0;
	Vector3 DragOffset = 0;
	Vector3 PreviousLocation;
	Model* ArrowsModel = nullptr;
	size_t SelectedTab = 0;
	struct Tab
	{
		size_t Index;
		std::string Name;
		bool CanBeClosed;
		std::string Type;
	};
	std::vector<Tab> Tabs = { Tab(0, "Viewport", false, "jscn")};
	std::vector<EditorTab*> TabInstances;
public:

	static Viewport* ViewportInstance;
	WorldObject* PreviousSelectedObject = nullptr;
	std::vector<WorldObject*> SelectedObjects;

	Viewport(Vector3* Colors, Vector2 Position, Vector2 Scale);
	void ClearSelectedObjects();
	void OpenTab(size_t TabID, std::string File);
	void OnButtonClicked(int Index);

	void UpdateTabBar();

	FramebufferObject* OutlineBuffer = nullptr, *ArrowsBuffer = nullptr;

	void Save() override;
	void Load(std::string File) override;
	void UpdateLayout() override;
	void Tick() override;
};
#endif