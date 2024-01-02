#if !SERVER
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
	if (GetOrientation() == UIBox::Orientation::Vertical)
	{
		float DesiredSize = 0;
		for (UIBox* i : Children)
		{
			DesiredSize += i->UpPadding + i->DownPadding + std::max({ i->GetUsedSize().Y, i->GetMinSize().Y, 0.0f });
		}
		return DesiredSize;
	}
	float DesiredSize = 0;
	for (UIBox* i : Children)
	{
		DesiredSize = std::max(i->UpPadding + i->DownPadding + std::max({ i->GetUsedSize().Y, i->GetMinSize().Y, 0.0f }), DesiredSize);
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
			ScrollBarBackground = new UIButton(UIBox::Orientation::Vertical, 0, 0.25f, nullptr, 0);
			ScrollBarBackground->ParentOverride = this;
			ScrollBarBackground->SetBorder(UIBox::BorderType::DarkenedEdge, 0.2f);
			ScrollBarBackground->SetPosition(OffsetPosition + Vector2(Size.X - ScrollBarBackground->GetUsedSize().X, 0));
#if EDITOR
			ScrollBar = new UIBackground(UIBox::Orientation::Horizontal, 0, 0.55f, Vector2(0.01f, 0.1f));
#else
			ScrollBar = new UIBackground(UIBox::Orientation::Horizontal, 0, 0.75f, Vector2(0.01f, 0.1f));
#endif
			ScrollBarBackground->AddChild(ScrollBar);
			ScrollBar->SetBorder(UIBox::BorderType::Rounded, 0.25);
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
	if (ScrollBarBackground)
	{
		ScrollBarBackground->IsVisible = VisibleInHierarchy && DesiredMaxScroll > Size.Y;
	}
	if (ScrollBar && VisibleInHierarchy)
	{
		ScrollBarBackground->SetMinSize(Vector2(0.015f, GetUsedSize().Y));
		ScrollBarBackground->SetPosition(OffsetPosition + Vector2(Size.X - ScrollBarBackground->GetUsedSize().X, 0));

		float ScrollPercentage = ScrollClass.Percentage / ScrollClass.MaxScroll;

		if (DesiredMaxScroll <= Size.Y)
		{
			ScrollBar->SetMinSize(Vector2(0.01f, Size.Y - 0.005f));
			ScrollBar->SetPadding(0.0025f);
			ScrollPercentage = 0;
		}
		else
		{
			ScrollBar->SetMinSize(Vector2(0.01f, Size.Y / (DesiredMaxScroll / Size.Y)));

			ScrollBar->SetPadding(std::max((ScrollPercentage * Size.Y) - (ScrollPercentage * ScrollBar->GetUsedSize().Y) - 0.005f, 0.0025f),
				0.0025f,
				0.0025f,
				0.0025f);
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


UIScrollBox::UIScrollBox(Orientation BoxOrientation, Vector2 Position, bool DisplayScrollBar) : UIBox(BoxOrientation, Position)
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
#endif