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

class UIButton;

namespace UI
{
	std::set<UIBox*> ElementsToUpdate;
	std::vector<UIBox*> UIElements;
	bool RequiresRedraw = true;
	unsigned int UIBuffer = 0;
	unsigned int UITextures[2] = {0, 0};
}

UIBox* UIBox::SetSizeMode(E_SizeMode NewMode)
{
	if (SizeMode != NewMode)
	{
		SizeMode = NewMode;
		InvalidateLayout();
	}
	return this;
}

UIBox::UIBox(bool Horizontal, Vector2 Position)
{
	this->Position = Position;
	this->Size = Size;
	this->ChildrenHorizontal = Horizontal;
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
				Parent->Children.erase(Parent->Children.begin() + i);
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
#if EDITOR
	if (!Editor::DraggingTab && IsHovered() && HasMouseCollision && IsVisibleInHierarchy())
#else
	if (IsHovered() && HasMouseCollision && IsVisibleInHierarchy())
#endif
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

UIBox* UIBox::SetBorder(E_BorderType Type, float Size)
{
	if (BorderType != Type || Size != BorderRadius)
	{
		BorderType = Type;
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
	Editor::CurrentUI->OnResized();
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
			(size_t)Graphics::WindowResolution.X * 2,
			(size_t)Graphics::WindowResolution.Y * 2,
			0,
			GL_RGBA,
			GL_FLOAT,
			NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, UI::UITextures[i], 0
		);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
		UpdateSelfAndChildren();
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

UIBox* UIBox::SetHorizontal(bool IsHorizontal)
{
	if (IsHorizontal != ChildrenHorizontal)
	{
		ChildrenHorizontal = IsHorizontal;
		InvalidateLayout();
	}
	return this;
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

	Update();
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
		if (ChildrenHorizontal)
		{
			Size.X += c->Size.X + c->LeftPadding + c->RightPadding;
			Size.Y = std::max(Size.Y, c->Size.Y + c->UpPadding + c->DownPadding);
		}
		else
		{
			Size.Y += c->Size.Y + c->UpPadding + c->DownPadding;
			Size.X = std::max(Size.X, c->Size.X + c->LeftPadding + c->RightPadding);
		}
	}

	Vector2 AdjustedMinSize = MinSize;
	Vector2 AdjustedMaxSize = MaxSize;
	if (SizeMode == E_PIXEL_RELATIVE)
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

	float ChildrenSize = 0;

	if (Align == E_CENTERED)
	{
		for (auto c : Children)
		{
			ChildrenSize += ChildrenHorizontal ? (c->Size.X + c->LeftPadding + c->RightPadding) : (c->Size.Y + c->UpPadding + c->DownPadding);
		}
	}


	for (auto c : Children)
	{
		if (Align == E_CENTERED)
		{
			if (ChildrenHorizontal)
			{
				c->OffsetPosition = OffsetPosition + Vector2(Size.X / 2 - ChildrenSize / 2 + c->LeftPadding, c->DownPadding);
				Offset += c->Size.X + c->LeftPadding + c->RightPadding;
			}
			else
			{
				c->OffsetPosition = OffsetPosition + Vector2(c->LeftPadding, Size.Y / 2 - ChildrenSize / 2 + c->DownPadding);
				Offset += c->Size.Y + c->DownPadding + c->UpPadding;
			}
		}
		else
		{
			if (ChildrenHorizontal)
			{
				if (Align == E_REVERSE)
				{
					c->OffsetPosition = OffsetPosition + Vector2(Size.X - Offset - c->Size.X - c->RightPadding, c->DownPadding);
				}
				else
				{
					c->OffsetPosition = OffsetPosition + Vector2(Offset + c->LeftPadding, c->DownPadding);
				}
				Offset += c->Size.X + c->LeftPadding + c->RightPadding;
			}
			else
			{
				if (Align == E_REVERSE)
				{
					c->OffsetPosition = OffsetPosition + Vector2(c->LeftPadding, Size.Y - Offset - c->Size.Y - c->UpPadding);
				}
				else
				{
					c->OffsetPosition = OffsetPosition + Vector2(c->LeftPadding, Offset + c->DownPadding);
				}
				Offset += c->Size.Y + c->DownPadding + c->UpPadding;
			}
		}
	}
	for (auto c : Children)
	{
		c->UpdatePosition();
		c->Update();
		if (c->TryFill)
		{
			if (ChildrenHorizontal)
			{
				c->Size.Y = Size.Y - (c->UpPadding + c->DownPadding);
				c->Size = c->Size.Clamp(c->MinSize, c->MaxSize);
			}
			else
			{
				c->Size.X = Size.X - (c->LeftPadding + c->RightPadding);
				c->Size = c->Size.Clamp(c->MinSize, c->MaxSize);
			}
		}
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
		if (elem->ShouldBeTicked)
		{
			elem->Tick();
		}
		if (!elem->Parent)
		{
			elem->UpdateHoveredState();
		}
	}
	UI::HoveredBox = UI::NewHoveredBox;
	if (UI::RequiresRedraw)
	{
		UI::RequiresRedraw = false;
		for (UIBox* elem : UI::ElementsToUpdate)
		{
			elem->UpdateSelfAndChildren();
		}
		UI::ElementsToUpdate.clear();
		glViewport(0, 0, (size_t)Graphics::WindowResolution.X * 2, (size_t)Graphics::WindowResolution.Y * 2);
		glEnable(GL_BLEND);
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
	if (!Parent) return IsVisible;
	if (IsVisible) return Parent->IsVisibleInHierarchy();
	return false;
}

namespace UI
{
	UIBox* HoveredBox = nullptr;
	UIBox* NewHoveredBox = nullptr;
}
