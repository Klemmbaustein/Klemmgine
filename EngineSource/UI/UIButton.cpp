#include "UIButton.h"
#include <Math/Math.h>
#include <Engine/Input.h>
#include <Engine/Log.h>
#include <Rendering/VertexBuffer.h>
#include <GL/glew.h>
#include <Engine/Application.h>

#if EDITOR
//extern bool UserDraggingButton;
#endif

void UIButton::ScrollTick(Shader* UsedShader)
{
	if (CurrentScrollObject != nullptr)
	{
		glUniform3f(glGetUniformLocation(UsedShader->GetShaderID(), "u_offset"), -CurrentScrollObject->Percentage, CurrentScrollObject->Position.Y, CurrentScrollObject->Position.Y - CurrentScrollObject->Scale.Y);
	}
	else
		glUniform3f(glGetUniformLocation(UsedShader->GetShaderID(), "u_offset"), 0, -1000, 1000);
}

void UIButton::MakeGLBuffers()
{
	std::vector<Vertex> Vertices;
	Vertex vertex1;
	vertex1.Position.x = 0;
	vertex1.Position.y = 0;
	vertex1.Position.z = 0;
	vertex1.U = 0;
	vertex1.V = 0;
	Vertices.push_back(vertex1);

	Vertex vertex2;
	vertex2.Position.x = 1;
	vertex2.Position.y = 0;
	vertex2.Position.z = 0;
	vertex2.U = 1;
	vertex2.V = 0;
	Vertices.push_back(vertex2);

	Vertex vertex3;
	vertex3.Position.x = 0;
	vertex3.Position.y = 1;
	vertex3.Position.z = 0;
	vertex3.U = 0;
	vertex3.V = 1;
	Vertices.push_back(vertex3);

	Vertex vertex4;
	vertex4.Position.x = 1;
	vertex4.Position.y = 1;
	vertex4.Position.z = 0;
	vertex4.U = 1;
	vertex4.V = 1;
	Vertices.push_back(vertex4);
	ButtonVertexBuffer = new VertexBuffer(Vertices.data(), Vertices.size());
}

void UIButton::Tick()
{
	if (!IsVisible)
	{
		return;
	}
	ButtonColorMultiplier = 1.0f;
	if (UI::HoveredButton == this && !IsHovered)
	{
		RedrawUI();
		IsHovered = true;
	}
	if (IsHovered && UI::HoveredButton != this)
	{
		RedrawUI();
		IsHovered = false;
	}
	if (CurrentScrollObject != nullptr)
	{
		Offset.Y = CurrentScrollObject->Percentage;
	}


	if (Maths::IsPointIn2DBox(OffsetPosition + Offset, OffsetPosition + Size + Offset, Input::MouseLocation) // If the mouse is on top of the button
		&& (!CurrentScrollObject || // Check if we have a scroll object
			Maths::IsPointIn2DBox(CurrentScrollObject->Position - CurrentScrollObject->Scale, CurrentScrollObject->Position, Input::MouseLocation))) // do some very questionable math to check if the mouse is inside the scroll area
	{
		UI::NewHoveredButton = this;
		ButtonColorMultiplier = 0.8f;
	}
	else if (IsPressed && UI::HoveredButton != this)
	{
		IsSelected = false;
		IsPressed = false;
		RedrawUI();
		if (CanBeDragged && ParentUI)
		{
			if (ParentUI) Application::ButtonEvents.insert(ButtonEvent(nullptr, ParentUI, ButtonIndex, true));
		}
	}

	if (IsSelected)
	{
		ButtonColorMultiplier = 0.6f;
	}

	if (UI::HoveredButton == this)
	{
		if (Input::IsLMBDown)
		{
			ButtonColorMultiplier = 0.5f;
			if (!IsPressed)
			{
				RedrawUI();
				IsPressed = true;
			}
		}
		else if (IsPressed)
		{
#if EDITOR
			if (!NeedsToBeSelected || IsSelected || false)
#else
			if (!NeedsToBeSelected || IsSelected)
#endif
			{
				if (ParentUI) Application::ButtonEvents.insert(ButtonEvent(nullptr, ParentUI, ButtonIndex));
				if (Parent || ParentOverride)
				{
					Application::ButtonEvents.insert(ButtonEvent(ParentOverride ? ParentOverride : Parent, nullptr, ButtonIndex));
				}
				IsPressed = false;
				IsSelected = false;
				RedrawUI();
			}
			else
			{
				IsPressed = false;
				IsSelected = true;
			}
		}
	}
	else if (Input::IsLMBDown)
	{
		IsSelected = false;
	}
}


UIButton* UIButton::SetOpacity(float NewOpacity)
{
	if (NewOpacity != Opacity)
	{
		Opacity = NewOpacity;
		RedrawUI();
	}
	return this;
}

float UIButton::GetOpacity()
{
	return Opacity;
}

void UIButton::SetCanBeDragged(bool NewCanBeDragged)
{
	CanBeDragged = NewCanBeDragged;
}

bool UIButton::GetIsSelected()
{
	return IsSelected;
}

void UIButton::SetNeedsToBeSelected(bool NeedsToBeSelected)
{
	this->NeedsToBeSelected = NeedsToBeSelected;
}

bool UIButton::GetIsHovered()
{
	return IsHovered;
}

bool UIButton::GetIsPressed()
{
	return IsPressed;
}

int UIButton::GetIndex()
{
	return ButtonIndex;
}

UIButton* UIButton::SetUseTexture(bool UseTexture, unsigned int TextureID)
{
	this->UseTexture = UseTexture;
	this->TextureID = TextureID;
	return this;
}

UIButton* UIButton::SetColor(Vector3 NewColor)
{
	if (NewColor != Color)
	{
		Color = NewColor;
		RedrawUI();
	}
	return this;
}

Vector3 UIButton::GetColor()
{
	return Color;
}

UIButton::UIButton(bool Horizontal, Vector2 Position, Vector3 Color, UICanvas* UI, int ButtonIndex, Shader* ButtonShader) : UIBox(Horizontal, Position)
{
	this->ButtonShader = ButtonShader;
	this->ButtonIndex = ButtonIndex;
	this->ParentUI = UI;
	this->Color = Color;
	MakeGLBuffers();
}

UIButton::~UIButton()
{
	delete ButtonVertexBuffer;
}

void UIButton::Update()
{
}

void UIButton::Draw()
{
	ButtonShader->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureID);
	ButtonVertexBuffer->Bind();
	ScrollTick(ButtonShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureID);
	glUniform4f(glGetUniformLocation(ButtonShader->GetShaderID(), "u_transform"), OffsetPosition.X, OffsetPosition.Y, Size.X, Size.Y);
	glUniform4f(glGetUniformLocation(ButtonShader->GetShaderID(), "u_color"),
		ButtonColorMultiplier * Color.X, ButtonColorMultiplier * Color.Y, ButtonColorMultiplier * Color.Z, 1.f);
	glUniform1f(glGetUniformLocation(ButtonShader->GetShaderID(), "u_opacity"), Opacity);
	glUniform1i(glGetUniformLocation(ButtonShader->GetShaderID(), "u_borderType"), BorderType);
	glUniform1f(glGetUniformLocation(ButtonShader->GetShaderID(), "u_borderScale"), BorderRadius / 20.0f);
	glUniform1f(glGetUniformLocation(ButtonShader->GetShaderID(), "u_aspectratio"), Graphics::AspectRatio);

	if (UseTexture)
		glUniform1i(glGetUniformLocation(ButtonShader->GetShaderID(), "u_usetexture"), 1);
	else
		glUniform1i(glGetUniformLocation(ButtonShader->GetShaderID(), "u_usetexture"), 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	ButtonVertexBuffer->Unbind();
}
