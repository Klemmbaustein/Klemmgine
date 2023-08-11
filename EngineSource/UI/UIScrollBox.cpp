#include "UIScrollBox.h"
#include <UI/Default/ScrollObject.h>
#include <Engine/Log.h>
#include <UI/UIButton.h>
#include <UI/UIBackground.h>
#include <Engine/Input.h>
#include <cmath>
#include <Engine/EngineError.h>

bool UIScrollBox::IsDraggingScrollBox = false;

float UIScrollBox::GetDesiredChildrenSize()
{
	float DesiredSize = 0;
	for (UIBox* i : Children)
	{
		DesiredSize += i->UpPadding + i->DownPadding + std::max({ i->GetUsedSize().Y, i->GetMinSize().Y, 0.0f });
		if (!i->GetUsedSize().Y)
		{
			GetAbsoluteParent()->InvalidateLayout();
		}
	}
	return DesiredSize;
}

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

UIBackground* UIScrollBox::GetScrollBarSlider()
{
	return ScrollBar;
}

UIButton* UIScrollBox::GetScrollBarBackground()
{
	return ScrollBarBackground;
}

UIScrollBox* UIScrollBox::SetDisplayScrollBar(bool NewDisplay)
{
	if (NewDisplay != DisplayScrollBar)
	{
		DisplayScrollBar = NewDisplay;
		if (DisplayScrollBar)
		{
			ScrollBarBackground = new UIButton(false, 0, 0.25, nullptr, 0);
			ScrollBarBackground->ParentOverride = this;
			ScrollBarBackground->SetBorder(UIBox::E_DARKENED_EDGE, 0.2);
			ScrollBarBackground->Align = UIBox::E_REVERSE;
			ScrollBarBackground->SetPosition(OffsetPosition + Vector2(Size.X - ScrollBarBackground->GetUsedSize().X, 0));
#if EDITOR
			ScrollBar = new UIBackground(true, 0, 0.4, Vector2(0.01, 0.1));
#else
			ScrollBar = new UIBackground(true, 0, 0.75, Vector2(0.01, 0.1));
#endif
			ScrollBarBackground->AddChild(ScrollBar);
			ScrollBar->SetBorder(UIBox::E_ROUNDED, 0.25);
			ScrollBar->SetPadding(0);
		}
		else if (ScrollBar)
		{
			delete ScrollBarBackground;
		}
	}
	return this;
}

bool UIScrollBox::GetDiplayScrollBar()
{
	return DisplayScrollBar;
}

void UIScrollBox::Tick()
{
	ScrollClass.Active = UI::HoveredBox && (UI::HoveredBox == this || UI::HoveredBox->IsChildOf(this));
	CurrentScrollObject = nullptr;
	bool VisibleInHierarchy = IsVisibleInHierarchy();
	DesiredMaxScroll = GetDesiredChildrenSize();
	ScrollBarBackground->IsVisible = VisibleInHierarchy;
	if (ScrollBar && VisibleInHierarchy)
	{
		ScrollBarBackground->SetMinSize(Vector2(0.015, GetUsedSize().Y));
		ScrollBarBackground->SetPosition(OffsetPosition + Vector2(Size.X - ScrollBarBackground->GetUsedSize().X, 0));

		float ScrollPercentage = ScrollClass.Percentage / ScrollClass.MaxScroll;

		if (DesiredMaxScroll <= Size.Y)
		{
			ScrollBar->SetMinSize(Vector2(0.01, Size.Y - 0.005));
			ScrollBar->SetPadding(0.0025);
			ScrollPercentage = 0;
		}
		else
		{
			ScrollBar->SetMinSize(Vector2(0.01, Size.Y / (DesiredMaxScroll / Size.Y)));

			ScrollBar->SetPadding(std::max((ScrollPercentage * Size.Y) - (ScrollPercentage * ScrollBar->GetUsedSize().Y) - 0.005f, 0.0025f),
				0.0025,
				0.0025,
				0.0025);
		}
		if ((ScrollBarBackground->GetIsPressed() && !IsDraggingScrollBox) || IsDragging && ScrollClass.MaxScroll)
		{
			float MousePos = (-Input::MouseLocation.Y + OffsetPosition.Y) / Size.Y + 1 + (ScrollPercentage * ScrollBar->GetUsedSize().Y);
			if (!IsDragging && std::abs(MousePos - ScrollPercentage) < ScrollBar->GetUsedSize().Y)
			{
				DraggingDelta = MousePos - ScrollPercentage;
			}
			else
			{
				ScrollClass.Percentage = std::min(std::max(MousePos - DraggingDelta, 0.0f), 1.0f) * ScrollClass.MaxScroll;
			}
			IsDragging = true;
			IsDraggingScrollBox = true;
		}
	}
	if (!Input::IsLMBDown)
	{
		IsDraggingScrollBox = false;
		IsDragging = false;
	}
}

UIScrollBox* UIScrollBox::SetMaxScroll(float NewMaxScroll)
{
	MaxScroll = NewMaxScroll;
	Update();
	return this;
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
	float ActualMaxScroll = MaxScroll;
	DesiredMaxScroll = MaxScroll + Size.Y;
	if (MaxScroll == -1)
	{
		DesiredMaxScroll = GetDesiredChildrenSize();
		ActualMaxScroll = std::max(DesiredMaxScroll - Size.Y, 0.0f);
	}
	ScrollClass = ScrollObject(OffsetPosition, Size, ActualMaxScroll);
	ScrollClass.Percentage = Progress;
	ScrollClass.Speed = Speed;
	UpdateScrollObjectOfObject(this);


	//Tick();
}

void UIScrollBox::UpdateTickState()
{
	for (auto c : Children)
	{
		c->UpdateTickState();
	}
	ShouldBeTicked = true;
}


UIScrollBox::UIScrollBox(bool Horizontal, Vector2 Position, bool DisplayScrollBar) : UIBox(Horizontal, Position)
{
	this->MaxScroll = MaxScroll;
	this->HasMouseCollision = true;
	SetDisplayScrollBar(DisplayScrollBar);
	Update();
}

UIScrollBox::~UIScrollBox()
{
	SetDisplayScrollBar(false);
}
