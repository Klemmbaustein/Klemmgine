#include "UIScrollBox.h"
#include <UI/Default/ScrollObject.h>
#include <Engine/Log.h>

void UIScrollBox::UpdateScrollObjectOfObject(UIBox* o)
{
	o->CurrentScrollObject = &ScrollClass;
	for (auto c : o->Children)
	{
		UpdateScrollObjectOfObject(c);
	}
}

ScrollObject* UIScrollBox::GetScrollObject()
{
	return &ScrollClass;
}

void UIScrollBox::SetMaxScroll(float NewMaxScroll)
{
	MaxScroll = NewMaxScroll;
	Update();
}

float UIScrollBox::GetMaxScroll()
{
	return MaxScroll;
}

UIScrollBox* UIScrollBox::SetScrollSpeed(float NewScrollSpeed)
{
	ScrollClass.Speed = NewScrollSpeed;
	return this;
}

float UIScrollBox::GetScrollSpeed()
{
	return ScrollClass.Speed;
}

void UIScrollBox::Update()
{
	float Progress = ScrollClass.Percentage;
	float Speed = ScrollClass.Speed;
	ScrollClass = ScrollObject(OffsetPosition, Size, MaxScroll);
	ScrollClass.Percentage = Progress;
	ScrollClass.Speed = Speed;
	UpdateScrollObjectOfObject(this);
}


UIScrollBox::UIScrollBox(bool Horizontal, Vector2 Position, float MaxScroll) : UIBox(Horizontal, Position)
{
	this->MaxScroll = MaxScroll;
	Update();
}
