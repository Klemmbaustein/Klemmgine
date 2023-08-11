#pragma once
#include <UI/UIBox.h>

class ScrollObject;
class UIButton;
class UIBackground;

class UIScrollBox : public UIBox
{
	ScrollObject ScrollClass = ScrollObject(OffsetPosition, Size, 15);
	UIButton* ScrollBarBackground = nullptr;
	UIBackground* ScrollBar = nullptr;
	float MaxScroll = -1;
	bool DisplayScrollBar = false;
	bool IsDragging = false;
	float DraggingDelta = 0;

	float DesiredMaxScroll = 0;
	float GetDesiredChildrenSize();
	void UpdateScrollObjectOfObject(UIBox* o);
public:
	ScrollObject* GetScrollObject();
	static bool IsDraggingScrollBox;

	UIBackground* GetScrollBarSlider();
	UIButton* GetScrollBarBackground();

	UIScrollBox* SetDisplayScrollBar(bool NewDisplay);
	bool GetDiplayScrollBar();

	void Tick() override;
	UIScrollBox* SetMaxScroll(float NewMaxScroll);
	float GetMaxScroll();
	UIScrollBox* SetScrollSpeed(float NewScrollSpeed);
	float GetScrollSpeed();
	void Update() override;
	void UpdateTickState() override;
	UIScrollBox(bool Horizontal, Vector2 Position, bool DisplayScrollBar);
	~UIScrollBox();
};