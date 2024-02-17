#if !SERVER
#include "UIBox.h"
#include <Engine/Log.h>
#include <iostream>
#include <GL/glew.h>
#include <Rendering/Graphics.h>
#include <Math/Math.h>
#include <Engine/Input.h>

#if EDITOR
#include <UI/EditorUI/EditorUI.h>
#endif
#include <Engine/EngineError.h>
#include <Engine/Application.h>

class UIButton;

namespace UI
{
	std::set<UIBox*> ElementsToUpdate;
	std::vector<UIBox*> UIElements;
	bool RequiresRedraw = true;
	unsigned int UIBuffer = 0;
	unsigned int UITextures[2] = {0, 0};
}

std::string UIBox::GetAsString()
{
	return typeid(*this).name();
}

void UIBox::DebugPrintTree(uint8_t Depth)
{
	for (uint8_t i = 0; i < Depth; i++)
	{
		std::cout << "    ";
	}
	std::cout << GetAsString() << std::endl;

	for (UIBox* i : Children)
	{
		i->DebugPrintTree(Depth + 1);
	}
}

UIBox* UIBox::SetSizeMode(SizeMode NewMode)
{
	if (BoxSizeMode != NewMode)
	{
		BoxSizeMode = NewMode;
		InvalidateLayout();
	}
	return this;
}

UIBox::UIBox(Orientation BoxOritentation, Vector2 Position)
{
	this->Position = Position;
	this->Size = Size;
	this->ChildrenOrientation = BoxOritentation;
	InvalidateLayout();
	for (UIBox* elem : UI::UIElements)
	{
		if (elem == this)
		{
			throw 1;
		}
	}
	UI::UIElements.push_back(this);
}

UIBox::~UIBox()
{
	InvalidateLayout();
	DeleteChildren();
	if (UI::HoveredBox == this)
	{
		UI::HoveredBox = nullptr;
	}
	if (UI::NewHoveredBox == this)
	{
		UI::NewHoveredBox = nullptr;
	}
	for (unsigned int i = 0; i < UI::UIElements.size(); i++)
	{
		if (UI::UIElements[i] == this)
		{
			UI::UIElements.erase(UI::UIElements.begin() + i);
		}
	}
	UI::ElementsToUpdate.erase(this);
	if (Parent)
	{
		for (int i = 0; i < Parent->Children.size(); i++)
		{
			if (Parent->Children[i] == this)
			{
				Parent->Children.erase(Parent->Children.begin() + i--);
			}
		}
	}
}

void UIBox::Draw()
{
}

void UIBox::Tick()
{
}

void UIBox::UpdateTickState()
{
	for (auto c : Children)
	{
		c->UpdateTickState();
	}
	ShouldBeTicked = IsVisibleInHierarchy();
}

void UIBox::UpdateHoveredState()
{
	if (IsHovered() && HasMouseCollision && IsVisibleInHierarchy())
	{
		UI::NewHoveredBox = this;
	}
	for (UIBox* Child : Children)
	{
		Child->UpdateHoveredState();
	}
}

void UIBox::OnChildClicked(int Index)
{
}

UIBox* UIBox::SetBorder(BorderType NativeType, float Size)
{
	if (BoxBorder != NativeType || Size != BorderRadius)
	{
		BoxBorder = NativeType;
		BorderRadius = Size;
		InvalidateLayout();
	}
	return this;
}

void UIBox::ForceUpdateUI()
{
	if (UI::UIBuffer)
	{
		glDeleteFramebuffers(1, &UI::UIBuffer);
		glDeleteTextures(2, UI::UITextures);
	}
	UI::UIBuffer = 0;
	InitUI();
	for (auto i : UI::UIElements)
	{
		i->InvalidateLayout();
	}
#if EDITOR
	Application::EditorInstance->OnResized();
#endif
}

void UIBox::InitUI()
{
	glGenFramebuffers(1, &UI::UIBuffer);
	// create floating point color buffer
	glGenTextures(2, UI::UITextures);
	glBindFramebuffer(GL_FRAMEBUFFER, UI::UIBuffer);

	for (unsigned int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, UI::UITextures[i]);
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGBA16F,
			(size_t)Graphics::WindowResolution.X,
			(size_t)Graphics::WindowResolution.Y,
			0,
			GL_RGBA,
			GL_FLOAT,
			NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, UI::UITextures[i], 0
		);
	}
}

unsigned int* UIBox::GetUITextures()
{
	return UI::UITextures;
}
void UIBox::RedrawUI()
{
	UI::RequiresRedraw = true;
}

void UIBox::ClearUI()
{
	UI::ElementsToUpdate.clear();
	std::vector<UIBox*> elems = UI::UIElements;
	for (auto* elem : elems)
	{
		if (!elem->Parent)
		{
			delete elem;
		}
	}
	UI::UIElements.clear();
	UI::RequiresRedraw = true;
}

bool UIBox::IsHovered()
{
	Vector2 Offset;
	if (CurrentScrollObject)
	{
		Offset.Y = CurrentScrollObject->Percentage;

	}
	// If the mouse is on top of the box
	return Math::IsPointIn2DBox(OffsetPosition + Offset, OffsetPosition + Size + Offset, Input::MouseLocation)
		&& (!CurrentScrollObject // Check if we have a scroll object
			|| Math::IsPointIn2DBox( // do some very questionable math to check if the mouse is inside the scroll area
				CurrentScrollObject->Position - CurrentScrollObject->Scale,
				CurrentScrollObject->Position,
				Input::MouseLocation));
}

Vector2 UIBox::GetUsedSize()
{
	return Size;
}

bool UIBox::IsChildOf(UIBox* Parent)
{
	if (Parent == this->Parent)
	{
		return true;
	}
	if (!this->Parent)
	{
		return false;
	}
	return this->Parent->IsChildOf(Parent);
}

void UIBox::OnAttached()
{
}

UIBox* UIBox::SetMaxSize(Vector2 NewMaxSize)
{
	if (NewMaxSize != MaxSize)
	{
		MaxSize = NewMaxSize;
		InvalidateLayout();
	}
	return this;
}

Vector2 UIBox::GetMaxSize()
{
	return MaxSize;
}

UIBox* UIBox::SetMinSize(Vector2 NewMinSize)
{
	if (NewMinSize != MinSize)
	{
		MinSize = NewMinSize;
		InvalidateLayout();
	}
	return this;
}

Vector2 UIBox::GetMinSize()
{
	return MinSize;
}

UIBox* UIBox::SetPosition(Vector2 NewPosition)
{
	if (NewPosition != Position)
	{
		Position = NewPosition;
		InvalidateLayout();
	}
	return this;
}

Vector2 UIBox::GetPosition()
{
	return OffsetPosition;
}

UIBox* UIBox::SetPadding(float Up, float Down, float Left, float Right)
{
	if (Up != UpPadding || Down != DownPadding || Left != LeftPadding || Right != RightPadding)
	{
		UpPadding = Up;
		DownPadding = Down;
		LeftPadding = Left;
		RightPadding = Right;
		InvalidateLayout();
	}
	return this;
}

UIBox* UIBox::SetPadding(float AllDirs)
{
	if (AllDirs != UpPadding || AllDirs != DownPadding || AllDirs != LeftPadding || AllDirs != RightPadding)
	{
		UpPadding = AllDirs;
		DownPadding = AllDirs;
		LeftPadding = AllDirs;
		RightPadding = AllDirs;
		InvalidateLayout();
	}
	return this;
}

UIBox* UIBox::SetTryFill(bool NewTryFill)
{
	if (TryFill != NewTryFill)
	{
		TryFill = NewTryFill;
		InvalidateLayout();
	}
	return this;
}

UIBox* UIBox::SetPaddingSizeMode(SizeMode NewSizeMode)
{
	if (NewSizeMode != PaddingSizeMode)
	{
		PaddingSizeMode = NewSizeMode;
		InvalidateLayout();
	}
	return this;
}

UIBox* UIBox::SetOrientation(Orientation NewOrientation)
{
	if (NewOrientation != ChildrenOrientation)
	{
		ChildrenOrientation = NewOrientation;
		InvalidateLayout();
	}
	return this;
}

UIBox::Orientation UIBox::GetOrientation()
{
	return ChildrenOrientation;
}

bool UIBox::GetTryFill()
{
	return TryFill;
}

void UIBox::Update()
{
}

void UIBox::UpdateSelfAndChildren()
{
	UpdateScale();
	UpdatePosition();
	UpdateScale();

	Update();
}

std::vector<UIBox*> UIBox::GetChildren()
{
	return Children;
}

void UIBox::SetRenderOrderIndex(size_t OrderIndex)
{
	UI::UIElements.erase(UI::UIElements.begin() + GetRenderOrderIndex());
	if (OrderIndex < UI::UIElements.size())
	{
		UI::UIElements.insert(UI::UIElements.begin() + OrderIndex, this);
	}
	else
	{
		UI::UIElements.push_back(this);
	}
}

size_t UIBox::GetRenderOrderIndex()
{
	for (size_t i = 0; i < UI::UIElements.size(); i++)
	{
		if (UI::UIElements[i] == this)
		{
			return i;
		}
	}
	return 0;
}

void UIBox::UpdateScale()
{
	for (auto c : Children)
	{
		c->UpdateScale();
	}
	Size = 0;
	for (auto c : Children)
	{
		if (ChildrenOrientation == Orientation::Horizontal)
		{
			Size.X += c->Size.X + GetLeftRightPadding(c).X + GetLeftRightPadding(c).Y;
			if (!c->TryFill)
			{
				Size.Y = std::max(Size.Y, c->Size.Y + c->UpPadding + c->DownPadding);
			}
		}
		else
		{
			Size.Y += c->Size.Y + c->UpPadding + c->DownPadding;
			if (!c->TryFill)
			{
				Size.X = std::max(Size.X, c->Size.X + GetLeftRightPadding(c).X + GetLeftRightPadding(c).Y);
			}
		}
	}

	if (TryFill && Parent)
	{
		if (Parent->ChildrenOrientation == Orientation::Horizontal)
		{
			Size.Y = Parent->Size.Y - (UpPadding + DownPadding);
		}
		else
		{
			Size.X = Parent->Size.X - (GetLeftRightPadding(this).X + GetLeftRightPadding(this).Y);
		}
	}


	Vector2 AdjustedMinSize = MinSize;
	Vector2 AdjustedMaxSize = MaxSize;
	if (BoxSizeMode == SizeMode::PixelRelative)
	{
		AdjustedMinSize.X /= Graphics::AspectRatio;
		AdjustedMaxSize.X /= Graphics::AspectRatio;
	}

	Size = Size.Clamp(AdjustedMinSize, AdjustedMaxSize);
	for (auto c : Children)
	{
		c->UpdateScale();
	}
}

void UIBox::UpdatePosition()
{
	float Offset = 0;

	if (!Parent)
	{
		OffsetPosition = Position;
	}
	
	Align PrimaryAlign = ChildrenOrientation == Orientation::Horizontal ? HorizontalBoxAlign : VerticalBoxAlign;

	float ChildrenSize = 0;

	if (PrimaryAlign == Align::Centered)
	{
		for (auto c : Children)
		{
			Vector2 LeftRight = GetLeftRightPadding(c);
			ChildrenSize += ChildrenOrientation == Orientation::Horizontal 
				? (c->Size.X + LeftRight.X + LeftRight.Y)
				: (c->Size.Y + c->UpPadding + c->DownPadding);
		}
	}


	for (auto c : Children)
	{
		if (PrimaryAlign == Align::Centered)
		{
			if (ChildrenOrientation == Orientation::Horizontal)
			{
				c->OffsetPosition = OffsetPosition + Vector2(Size.X / 2 - ChildrenSize / 2 + GetLeftRightPadding(c).X, c->GetVerticalOffset());
				Offset += c->Size.X + GetLeftRightPadding(c).X + GetLeftRightPadding(c).Y;
			}
			else
			{
				c->OffsetPosition = OffsetPosition + Vector2(c->GetHorizontalOffset(), Size.Y / 2 - ChildrenSize / 2 + c->DownPadding);
				Offset += c->Size.Y + c->DownPadding + c->UpPadding;
			}
		}
		else
		{
			if (ChildrenOrientation == Orientation::Horizontal)
			{	
				if (PrimaryAlign == Align::Reverse)
				{
					c->OffsetPosition = OffsetPosition + Vector2(Size.X - Offset - c->Size.X - GetLeftRightPadding(c).Y, c->GetVerticalOffset());
				}
				else
				{
					c->OffsetPosition = OffsetPosition + Vector2(Offset + GetLeftRightPadding(c).X, c->GetVerticalOffset());
				}
				Offset += c->Size.X + GetLeftRightPadding(c).X + GetLeftRightPadding(c).Y;
			}
			else
			{
				if (PrimaryAlign == Align::Reverse)
				{
					c->OffsetPosition = OffsetPosition + Vector2(c->GetHorizontalOffset(), Size.Y - Offset - c->Size.Y - c->UpPadding);
				}
				else
				{
					c->OffsetPosition = OffsetPosition + Vector2(c->GetHorizontalOffset(), Offset + c->DownPadding);
				}
				Offset += c->Size.Y + c->DownPadding + c->UpPadding;
			}
		}
	}
	for (auto c : Children)
	{
		c->UpdatePosition();
		c->Update();
	}
}

void UIBox::InvalidateLayout()
{
	UI::RequiresRedraw = true;

	if (Parent)
	{
		Parent->InvalidateLayout();
	}
	else
	{
		UI::ElementsToUpdate.insert(this);
	}

}

UIBox* UIBox::AddChild(UIBox* NewChild)
{
	if (!NewChild->Parent)
	{
		NewChild->Parent = this;
		Children.push_back(NewChild);
		NewChild->OnAttached();
		InvalidateLayout();
	}
	else
	{
		Log::Print("Warning: Attached an UI object twice!", Vector3(1, 1, 0));
	}
	return this;
}

UIBox* UIBox::GetAbsoluteParent()
{
	if (Parent != nullptr)
	{
		return Parent->GetAbsoluteParent();
	}
	return this;
}

UIBox* UIBox::SetHorizontalAlign(Align NewAlign)
{
	HorizontalBoxAlign = NewAlign;
	return this;
}

UIBox* UIBox::SetVerticalAlign(Align NewAlign)
{
	VerticalBoxAlign = NewAlign;
	return this;
}

void UIBox::DrawAllUIElements()
{
	UI::NewHoveredBox = nullptr;

	for (UIBox* elem : UI::UIElements)
	{
		if (elem->IsVisible != elem->PrevIsVisible)
		{
			UI::RequiresRedraw = true;
			elem->PrevIsVisible = elem->IsVisible;
		}
		if (!elem->Parent)
		{
			elem->UpdateHoveredState();
		}
	}
	UI::HoveredBox = UI::NewHoveredBox;
	for (UIBox* elem : UI::UIElements)
	{
		if (elem->ShouldBeTicked)
		{
			elem->Tick();
		}
	}
	if (UI::RequiresRedraw)
	{
		UI::RequiresRedraw = false;
		for (UIBox* elem : UI::ElementsToUpdate)
		{
			elem->UpdateSelfAndChildren();
		}
		UI::ElementsToUpdate.clear();
		glViewport(0, 0, (size_t)Graphics::WindowResolution.X, (size_t)Graphics::WindowResolution.Y);
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, UI::UIBuffer);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		for (UIBox* elem : UI::UIElements)
		{
			if (!elem->Parent)
			{
				elem->DrawThisAndChildren();
			}
		}
		glClearColor(0, 0, 0, 1);
		glViewport(0, 0, (size_t)Graphics::WindowResolution.X, (size_t)Graphics::WindowResolution.Y);
	}
}

float UIBox::GetVerticalOffset()
{
	float VerticalOffset = DownPadding;

	if (Parent->VerticalBoxAlign == Align::Reverse)
	{
		VerticalOffset = Parent->Size.Y - UpPadding - Size.Y;
	}
	else if (Parent->VerticalBoxAlign == Align::Centered)
	{
		VerticalOffset = std::lerp(Parent->Size.Y - UpPadding - Size.Y, DownPadding, 0.5f);
	}
	return VerticalOffset;
}

float UIBox::GetHorizontalOffset()
{
	float HorizontalOffset = GetLeftRightPadding(this).X;

	if (Parent->HorizontalBoxAlign == Align::Reverse)
	{
		HorizontalOffset = Parent->Size.X - GetLeftRightPadding(this).Y - Size.X;
	}
	else if (Parent->HorizontalBoxAlign == Align::Centered)
	{
		HorizontalOffset = Parent->Size.X / 2 - Size.X / 2;
	}
	return HorizontalOffset;
}

Vector2 UIBox::GetLeftRightPadding(UIBox* Target)
{
	if (Target->PaddingSizeMode == SizeMode::ScreenRelative)
	{
		return Vector2(Target->LeftPadding, Target->RightPadding);
	}
	return Vector2(Target->LeftPadding, Target->RightPadding) / Graphics::AspectRatio;
}

void UIBox::DrawThisAndChildren()
{
	for (auto c : Children)
	{
		c->UpdateTickState();
	}
	if (IsVisible)
	{
		Draw();
		for (auto c : Children)
		{
			c->DrawThisAndChildren();
		}
	}
}

void UIBox::DeleteChildren()
{
	while (Children.size() != 0)
	{
		delete Children[0];
	}
	Children.clear();
}

bool UIBox::IsVisibleInHierarchy()
{
	if (!Parent)
	{
		return IsVisible;
	}
	if (IsVisible) return Parent->IsVisibleInHierarchy();
	return false;
}

namespace UI
{
	UIBox* HoveredBox = nullptr;
	UIBox* NewHoveredBox = nullptr;
}
#endif