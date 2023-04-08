#pragma once
#include "UI/UIBackground.h"
#include "UI/Default/UICanvas.h"
#include <UI/UIfwd.h>

class EditorPanel : public UICanvas
{
private:
	bool IsPopup = false;
	bool IsDragged = false;
	bool IsDragHorizontal = false;
	bool IsMouseDown = false;
	bool IsDraggingAll = false;
	Vector2 InitialMousePosition = 0;
	Vector2 InitialScale = 0;
	Vector2 InitialPosition = 0;
	UIBox* MainBackground = nullptr;
protected:
	UIBackground* TabBackground;
	UIBackground* TitleBackground = nullptr;
	UIText* TitleText = nullptr;
	UIBackground* ButtonBackground = nullptr;
public:
	void SetScale(Vector2 NewScale);
	void SetPosition(Vector2 NewPosition);
	Vector3* UIColors;
	Vector2 Position, Scale;
	Vector2 MinSize;
	Vector2 MaxSize;
	EditorPanel(Vector3* UIColors, Vector2 Position, Vector2 Scale, Vector2 MinSize, Vector2 MaxSize = 9999, bool IsPopup = false, std::string Title = "");
	virtual ~EditorPanel();

	virtual void UpdateLayout() = 0;
		
	void UpdatePanel();
	void SetVisible(bool NewVisible)
	{
		if (NewVisible != TabBackground->IsVisible)
		{
			UIBox::RedrawUI();
			TabBackground->IsVisible = NewVisible;
		}
	}
};