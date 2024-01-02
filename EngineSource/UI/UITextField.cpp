#if !SERVER
#include "UITextField.h"
#include <UI/UIText.h>
#include <Math/Math.h>
#include <Engine/Input.h>
#include <Rendering/VertexBuffer.h>
#include <GL/glew.h>
#include <Engine/Log.h>
#include <Engine/Application.h>

void UITextField::Tick()
{
	if (!IsVisible) return;

	if (Size.X != 0)
	{
		TextObject->WrapDistance = std::max(Size.X * 1.75f, 0.1f);
	}
	else
	{
		TextObject->WrapDistance = 10;
	}
	ColorMultiplier = 1.f;
	if (UI::HoveredBox == this)
	{
		size_t Nearest = TextObject->GetNearestLetterAtLocation(Input::MouseLocation);
		if (!IsHovered)
		{
			RedrawUI();
		}
		IsHovered = true;
		ColorMultiplier = 0.8f;
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
			IsEdited = true;
			TextInput::PollForText = true;
			TextInput::Text = EnteredText;
			TextInput::TextIndex = (int)Nearest;
			TextFieldTimer = 0;
			IsPressed = false;
			RedrawUI();
		}
	}
	else
	{
		if (IsPressed)
		{
			IsPressed = false;
		}
		if (IsHovered)
		{
			IsHovered = false;
			RedrawUI();
		}
	}
	if (IsEdited)
	{
		EnteredText = TextInput::Text;
		if (!TextInput::PollForText)
		{
			IsEdited = false;
			if (ParentUI) Application::ButtonEvents.insert(ButtonEvent(nullptr, ParentUI, ButtonIndex));
			if (Parent || ParentOverride)
			{
				Application::ButtonEvents.insert(ButtonEvent(ParentOverride ? ParentOverride : Parent, nullptr, ButtonIndex));
			}
			RedrawUI();
		}
		if (!IsHovered && Input::IsLMBDown)
		{
			IsEdited = false;
			TextInput::PollForText = false;
			if (ParentUI) Application::ButtonEvents.insert(ButtonEvent(nullptr, ParentUI, ButtonIndex));
			if (Parent || ParentOverride)
			{
				Application::ButtonEvents.insert(ButtonEvent(ParentOverride ? ParentOverride : Parent, nullptr, ButtonIndex));
			}
			RedrawUI();
		}
	}
	std::string RendererdText = EnteredText;
	TextFieldTimer += Performance::DeltaTime;
	if (fmod(TextFieldTimer, 1) < 0.5f && IsEdited)
	{
		Vector2 NewPos = TextObject->GetLetterLocation(TextInput::TextIndex);
		if (NewPos != IBeamPosition)
		{
			TextFieldTimer = 0;
			IBeamPosition = NewPos;
			IBeamScale = Vector2(2.0f / Graphics::WindowResolution.X, 0.066f * TextSize);
			UIBox::RedrawUI();
		}
		if (!ShowIBeam)
		{
			UIBox::RedrawUI();
		}
		ShowIBeam = true;
	}
	else
	{
		if (ShowIBeam)
		{
			UIBox::RedrawUI();
		}
		ShowIBeam = false;
	}
	TextObject->SetColor(EnteredText.empty() && !IsEdited ? TextColor * Vector3(0.75f) : TextColor);
	TextObject->SetText(EnteredText.empty() && !IsEdited ? HintText : (IsEdited ? RendererdText : EnteredText));
}

Vector3 UITextField::GetColor() const
{
	return Color;
}

UITextField* UITextField::SetColor(Vector3 NewColor)
{
	if (NewColor != Color)
	{
		Color = NewColor;
		RedrawUI();
	}
	return this;
}

UITextField* UITextField::SetTextColor(Vector3 NewColor)
{
	TextColor = NewColor;
	return this;
}

Vector3 UITextField::GetTextColor() const
{
	return TextColor;
}

void UITextField::Edit()
{
	IsEdited = true;
	TextInput::PollForText = true;
	TextInput::Text = EnteredText;
	IsPressed = false;
	TextInput::TextIndex = (int)TextInput::Text.size();
	RedrawUI();
}

UITextField* UITextField::SetText(std::string NewText)
{
	if (NewText != EnteredText)
	{
		EnteredText = NewText;
		InvalidateLayout();
		if (IsEdited)
		{
			TextInput::Text = NewText;
		}
	}
	return this;
}

UITextField* UITextField::SetTextSize(float NewTextSize)
{
	if (NewTextSize != TextSize)
	{
		TextObject->SetTextSize(NewTextSize);
		TextSize = NewTextSize;
	}
	return this;
}

float UITextField::GetTextSize() const
{
	return TextSize;
}

std::string UITextField::GetText()
{
	return EnteredText;
}

bool UITextField::GetIsHovered() const
{
	return IsHovered;
}

bool UITextField::GetIsPressed() const
{
	return IsPressed;
}

UITextField::UITextField(Vector2 Position, Vector3 Color, UICanvas* UI, int ButtonIndex, TextRenderer* Renderer, Shader* ButtonShader)
	: UIBackground(UIBox::Orientation::Vertical, Position, Color, 0)
{
	this->ButtonIndex = ButtonIndex;
	this->ButtonShader = ButtonShader;
	this->ParentUI = UI;
	TextObject = new UIText(0, Vector3(1), HintText, Renderer);
	TextObject->SetTextSize(0.4f);
	TextObject->SetPadding(0.005f);
	
	HasMouseCollision = true;
	TextObject->Wrap = true;
	AddChild(TextObject);

}

UITextField::~UITextField()
{
	if (IsEdited)
	{
		IsEdited = false;
		EnteredText = TextInput::Text;
		TextInput::PollForText = false;
	}
}

void UITextField::DrawBackground()
{
	ButtonShader->Bind();
	BoxVertexBuffer->Bind();
	if (ShowIBeam)
	{
		ButtonShader->SetVector4("u_color", Vector4(TextColor, 1));
		ButtonShader->SetInt("u_borderType", (int)UIBox::BorderType::None);
		glUniform4f(glGetUniformLocation(ButtonShader->GetShaderID(), "u_transform"), IBeamPosition.X, IBeamPosition.Y, IBeamScale.X, IBeamScale.Y);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	BoxVertexBuffer->Unbind();
}
#endif