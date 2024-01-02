#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>

class WorldObject;
class Model;
class EditorTab;

/**
* @brief
* EditorPanel showing the scene view.
* 
* @ingroup Editor
*/
class Viewport : public EditorPanel
{
	bool ViewportLock = false;
	bool PressedLMB = false;
	Vector2 InitialMousePosition = 0;
	bool Dragging = false;
	Vector3 Axis;
	int BoxAxis = 1;
	bool FirstDragFrame = 0;
	Vector3 DragOffset = 0;
	Vector3 PreviousLocation;
	Model* ArrowsModel = nullptr;
	bool IsCopying = false;
public:
	static Viewport* ViewportInstance;
	WorldObject* PreviousSelectedObject = nullptr;
	size_t PreviousSelectedObjectSize = 0;

	Viewport(EditorPanel* Parent);
	void ClearSelectedObjects();
	virtual void OnItemDropped(DroppedItem Item) override;
	void OnButtonClicked(int Index);

	FramebufferObject* OutlineBuffer = nullptr, *ArrowsBuffer = nullptr;

	void OnResized() override;
	void Tick() override;
};
#endif