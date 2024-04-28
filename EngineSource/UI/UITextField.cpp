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
		TextObject->WrapDistance = 1;
	}
	ColorMultiplier = 1.f;
	if (UI::HoveredBox == this || Dragging)
	{
		size_t Nearest = TextObject->GetNearestLetterAtLocation(Input::MouseLocation);
		if (!IsHovered)
		{
			RedrawElement();
		}
		IsHovered = true;
		ColorMultiplier = 0.8f;

		// Double click selects all
		if (Input::IsLMBClicked && IsEdited && DoubleClickTimer < 0.25f && TextInput::TextIndex == Nearest)
		{
			TextInput::TextSelectionStart = 0;
			TextInput::TextIndex = (int)TextInput::Text.size();
			Dragging = true;
		}
		else if (Input::IsLMBDown && !(!Dragging && TextInput::PollForText && !IsEdited))
		{
			ColorMultiplier = 0.5f;
			TextInput::PollForText = true;
			TextInput::Text = EnteredText;
			TextInput::SetTextIndex((int)Nearest, !Dragging);
			DoubleClickTimer = 0;
			Dragging = true;
			TextFieldTimer = 0;
			IsEdited = true;
			if (!IsPressed)
			{
				RedrawElement();
				IsPressed = true;
			}
		}
		else if (IsPressed && !Dragging)
		{
			IsPressed = false;
			RedrawElement();
		}
	}
	else
	{
		if (IsPressed && !Dragging)
		{
			IsPressed = false;
		}
		if (IsHovered)
		{
			IsHovered = false;
			RedrawElement();
		}
	}

	if (!Input::IsLMBDown)
	{
		Dragging = false;
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
			RedrawElement();
		}
		if (!IsHovered && Input::IsLMBDown && !Dragging)
		{
			IsEdited = false;
			TextInput::PollForText = false;
			if (ParentUI) Application::ButtonEvents.insert(ButtonEvent(nullptr, ParentUI, ButtonIndex));
			if (Parent || ParentOverride)
			{
				Application::ButtonEvents.insert(ButtonEvent(ParentOverride ? ParentOverride : Parent, nullptr, ButtonIndex));
			}
			RedrawElement();
		}
	}
	std::string RendererdText = EnteredText;
	TextFieldTimer += Stats::DeltaTime;
	DoubleClickTimer += Stats::DeltaTime;
	Vector2 EditedTextPos = IsEdited ? TextObject->GetLetterLocation(TextInput::TextIndex) : 0;

	if (fmod(TextFieldTimer, 1) < 0.5f && IsEdited)
	{
		if (EditedTextPos != IBeamPosition)
		{
			TextFieldTimer = 0;
			IBeamPosition = EditedTextPos;
			IBeamScale = Vector2(2.0f / Graphics::WindowResolution.X, 0.075f * TextSize);
			RedrawElement();
		}
		if (!ShowIBeam)
		{
			RedrawElement();
		}
		ShowIBeam = true;
	}
	else
	{
		if (ShowIBeam)
		{
			RedrawElement();
		}
		ShowIBeam = false;
	}

	if (IsEdited)
	{
		TextHighlightPos = TextObject->GetLetterLocation(TextInput::TextSelectionStart);
		TextHighlightSize = Vector2(std::abs(EditedTextPos.X - TextHighlightPos.X), 0.075f * TextSize);

		float MinX = std::min(EditedTextPos.X, TextHighlightPos.X);
		TextHighlightPos.X = MinX;
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
		RedrawElement();
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
	TextInput::SetTextIndex((int)TextInput::Text.size(), true);
	RedrawElement();
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
	: UIBackground(UIBox::Orientation::Vertical, Position, Color, 0, ButtonShader)
{
	this->ButtonIndex = ButtonIndex;
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
	BackgroundShader->Bind();
	BoxVertexBuffer->Bind();

	if (IsEdited)
	{
		BackgroundShader->SetVector4("u_color", Vector4(Vector3(0, 0.25f, 1), 1));
		BackgroundShader->SetInt("u_borderType", (int)UIBox::BorderType::None);
		BackgroundShader->SetFloat("u_opacity", 0.5f);
		glUniform4f(glGetUniformLocation(BackgroundShader->GetShaderID(), "u_transform"), TextHighlightPos.X, TextHighlightPos.Y, TextHighlightSize.X, TextHighlightSize.Y);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	if (ShowIBeam)
	{
		BackgroundShader->SetVector4("u_color", Vector4(TextColor, 1));
		BackgroundShader->SetInt("u_borderType", (int)UIBox::BorderType::None);
		BackgroundShader->SetFloat("u_opacity", 1);
		glUniform4f(glGetUniformLocation(BackgroundShader->GetShaderID(), "u_transform"), IBeamPosition.X, IBeamPosition.Y, IBeamScale.X, IBeamScale.Y);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	BoxVertexBuffer->Unbind();
}
#endif