#if EDITOR
#pragma once
#include <UI/EditorUI/EditorTab.h>

class Viewport : public EditorTab
{
public:
	Viewport(Vector3* Colors, Vector2 Position, Vector2 Scale);

	void Save() override;
	void Load(std::string File) override;
	void UpdateLayout() override;
	void Tick() override;
};
#endif