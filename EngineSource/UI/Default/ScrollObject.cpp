#if !SERVER
#include <UI/Default/ScrollObject.h>
#include <Math/Math.h>
#include <Rendering/Graphics.h>
#include <Engine/Input.h>
#include <Engine/Log.h>
#include <iostream>
#include <UI/UIBox.h>


ScrollObject::ScrollObject(Vector2 Position, Vector2 Scale, float MaxScroll)
{
	this->Position = Position;
	this->Scale = Vector2() - Scale;
	UIBox::ScrollObjects.push_back(this);
	this->MaxScroll = MaxScroll;
}

ScrollObject::~ScrollObject()
{
	int i = 0;
	for (ScrollObject* s : UIBox::ScrollObjects)
	{
		if (s == this)
		{
			UIBox::ScrollObjects.erase(UIBox::ScrollObjects.begin() + i);
		}
		i++;
	}
}

void ScrollObject::ScrollUp()
{
	if (!Active)
	{
		return;
	}
	if (Math::IsPointIn2DBox(Position - Scale, Position, Input::MouseLocation))
	{
		Percentage += Speed / 100.f;
	}
	if (Percentage > MaxScroll)
	{
		Percentage = MaxScroll;
	}
	UIBox::RedrawArea(UIBox::RedrawBox{
		.Min = Position,
		.Max = Position - Scale,
		});
}

void ScrollObject::ScrollDown()
{
	if (!Active)
	{
		return;
	}
	if (Math::IsPointIn2DBox(Position - Scale, Position, Input::MouseLocation))
	{
		Percentage -= Speed / 100.f;
	}
	if (Percentage < 0)
		Percentage = 0;

	UIBox::RedrawArea(UIBox::RedrawBox{
		.Min = Position,
		.Max = Position - Scale,
		});
}
#endif