#pragma once
#include "UI/UIBackground.h"
#include "UI/Default/UICanvas.h"

class EditorTab : public UICanvas
{
protected:
	UIBackground* TabBackground;
	Vector3* UIColors;
public:
	EditorTab(Vector3* UIColors) : UICanvas()
	{
		this->UIColors = UIColors;
		TabBackground = new UIBackground(false, Vector2(-0.7, -0.59), Vector3(UIColors[0] * 0.9), Vector2(1.4, 1.3));
		TabBackground->IsVisible = false;
	}

	virtual void Load(std::string File) = 0;
	void SetVisible(bool NewVisible)
	{
		if (NewVisible != TabBackground->IsVisible)
		{
			UIBox::RedrawUI();
			TabBackground->IsVisible = NewVisible;
		}
	}
	virtual void Save() = 0;
};