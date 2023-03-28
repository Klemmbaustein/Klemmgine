#if EDITOR
#pragma once
#include <UI/Default/UICanvas.h>
#include <UI/UIBackground.h>

class EditorTab : public UICanvas
{
protected:
	Vector3* UIColors;
public:
	UIBackground* TabBackground = nullptr;
	virtual void Save() = 0;
	virtual void Load(std::string File) = 0;
	virtual void UpdateLayout();
	EditorTab(Vector3* UIColors);
};
#endif