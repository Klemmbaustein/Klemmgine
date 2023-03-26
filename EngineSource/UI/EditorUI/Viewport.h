#if EDITOR
#pragma once
#include <UI/EditorUI/EditorTab.h>

class WorldObject;

class Viewport : public EditorTab
{
	bool ViewportLock = false;
	bool PressedLMB = false;
	Vector2 InitialMousePosition = 0;


public:
	std::vector<WorldObject*> SelectedObjects;

	Viewport(Vector3* Colors, Vector2 Position, Vector2 Scale);
	void ClearSelectedObjects();


	FramebufferObject* OutlineBuffer = nullptr;

	void Save() override;
	void Load(std::string File) override;
	void UpdateLayout() override;
	void Tick() override;
};
#endif