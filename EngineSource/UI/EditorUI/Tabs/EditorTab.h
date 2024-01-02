#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>
#include <UI/UIBackground.h>

/**
* @brief
* EditorPanel that modifies/changes assets.
*/
class EditorTab : public EditorPanel
{
protected:

public:
	virtual void Save();
	virtual void Load(std::string File);
	EditorTab(EditorPanel* Parent, std::string Name, std::string File);
	EditorTab(Vector2 Position, Vector2 Scale, std::string Name, std::string File);
};
#endif