#pragma once
#include "UI/UIBackground.h"
#include "UI/Default/UICanvas.h"

class EditorPanel : public UICanvas
{
private:
	bool IsDragged = false;
	bool IsDragHorizontal = false;
	bool IsMouseDown = false;
	Vector2 InitialMousePosition = 0;
	Vector2 InitialScale = 0;
	Vector2 InitialPosition = 0;
protected:
	UIBackground* TabBackground;
public:
	void SetScale(Vector2 NewScale);
	void SetPosition(Vector2 NewPosition);
	Vector3* UIColors;
	Vector2 Position, Scale;
	Vector2 MinSize;
	Vector2 MaxSize;
	EditorPanel(Vector3* UIColors, Vector2 Position, Vector2 Scale, Vector2 MinSize, Vector2 MaxSize = 9999) : UICanvas()
	{
		this->UIColors = UIColors;

		Scale.X = std::max(Scale.X, MinSize.X);
		Scale.Y = std::max(Scale.Y, MinSize.Y);
		Scale.X = std::min(Scale.X, MaxSize.X);
		Scale.Y = std::min(Scale.Y, MaxSize.Y);

		this->MinSize = MinSize;
		this->MaxSize = MaxSize;

		TabBackground = new UIBackground(true, Position, Vector3(UIColors[0]), Scale);
		TabBackground->SetBorder(UIBox::E_DARKENED_EDGE, 0.2);
		this->Position = Position;
		this->Scale = Scale;
		//TabBackground->IsVisible = false;
	}

	virtual void UpdateLayout() = 0;
	virtual void Save() = 0;
	virtual void Load(std::string File) = 0;
		
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