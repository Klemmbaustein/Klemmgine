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

void UIScrollBox::Update()
{
	float Progress = ScrollClass.Percentage;
	ScrollClass = ScrollObject(OffsetPosition, Size, MaxScroll);
	ScrollClass.Percentage = Progress;
	UpdateScrollObjectOfObject(this);
}


UIScrollBox::UIScrollBox(bool Horizontal, Vector2 Position, float MaxScroll) : UIBox(Horizontal, Position)
{
	this->MaxScroll = MaxScroll;
	Update();
}
