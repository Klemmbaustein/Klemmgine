#if !SERVER
#include "UIButton.h"
#include <Math/Math.h>
#include <Engine/Input.h>
#include <Engine/Log.h>
#include <Rendering/VertexBuffer.h>
#include <GL/glew.h>
#include <Engine/Application.h>
#include <UI/UIScrollBox.h>

#if EDITOR
#include <UI/EditorUI/EditorUI.h>
#endif

void UIButton::Tick()
{
	if (!IsVisible)
	{
		return;
	}
	if (Input::IsLMBClicked && UI::HoveredBox != this)
	{
		ClickStartedOnButton = false;
	}
	else if (Input::IsLMBClicked && UI::HoveredBox == this)
	{
		ClickStartedOnButton = true;
	}
	if (Input::IsLMBDown && !ClickStartedOnButton)
	{
		return;
	}

	ColorMultiplier = 1.0f;
	if (UI::HoveredBox == this && !IsHovered)
	{
		RedrawUI();
		IsHovered = true;
	}
	if (IsHovered && UI::HoveredBox != this)
	{
		RedrawUI();
		IsHovered = false;
	}
	if (CurrentScrollObject != nullptr)
	{
		Offset.Y = CurrentScrollObject->Percentage;
	}

	if (UI::HoveredBox == this)
	{
		ColorMultiplier = 0.8f;
	}
	if (IsPressed && UI::HoveredBox != this && !UIScrollBox::IsDraggingScrollBox)
	{
		IsPressed = false;
		RedrawUI();
		if (CanBeDragged && ParentUI)
		{
			Application::ButtonEvents.insert(ButtonEvent(nullptr, ParentUI, ButtonIndex, true));
		}
	}


	if (UI::HoveredBox == this)
	{
		if (Input::IsLMBDown)
		{
			ColorMultiplier = 0.5f;
			if (!IsPressed)
			{
				RedrawUI();
				IsPressed = true;
			}
		}
		else if (IsPressed)
		{
			OnClicked();
			IsPressed = false;
			RedrawUI();
		}
	}
}

std::string UIButton::GetAsString()
{
	if (ParentUI)
	{
		return "UIButton (" + std::string(typeid(*ParentUI).name()) + ")";
	}
	return "UIButton (No parent)";
}

UIButton* UIButton::SetCanBeDragged(bool NewCanBeDragged)
{
	CanBeDragged = NewCanBeDragged;
	return this;
}

bool UIButton::GetIsHovered() const
{
	return IsHovered;
}

bool UIButton::GetIsPressed() const
{
	return IsPressed;
}

int UIButton::GetIndex() const
{
	return ButtonIndex;
}

UIButton::UIButton(Orientation BoxOrientation, Vector2 Position, Vector3 Color, UICanvas* UI, int ButtonIndex, Shader* ButtonShader)
	: UIBackground(BoxOrientation, Position, Color)
{
	this->ButtonIndex = ButtonIndex;
	this->ParentUI = UI;
	HasMouseCollision = true;
}

UIButton::~UIButton()
{
}

void UIButton::Update()
{
}

void UIButton::OnClicked()
{
	if (ParentUI)
	{
		Application::ButtonEvents.insert(ButtonEvent(nullptr, ParentUI, ButtonIndex));
	}
	if (Parent || ParentOverride)
	{
		Application::ButtonEvents.insert(ButtonEvent(ParentOverride ? ParentOverride : Parent, nullptr, ButtonIndex));
	}
}
#endif