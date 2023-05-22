#include "UITextField.h"
#include <UI/UIText.h>
#include <Math/Math.h>
#include <Engine/Input.h>
#include <Engine/Timer.h>
#include <Rendering/VertexBuffer.h>
#include <GL/glew.h>
#include <Engine/Log.h>
#include <Engine/Application.h>

void UITextField::ScrollTick(Shader* UsedShader)
{
	if (CurrentScrollObject != nullptr)
	{
		glUniform3f(glGetUniformLocation(UsedShader->GetShaderID(), "u_offset"),
			-CurrentScrollObject->Percentage, CurrentScrollObject->Position.Y, CurrentScrollObject->Position.Y - CurrentScrollObject->Scale.Y);
	}
	else
		glUniform3f(glGetUniformLocation(UsedShader->GetShaderID(), "u_offset"), 0, -1000, 1000);
}

void UITextField::MakeGLBuffers()
{
	if (ButtonVertexBuffer)
	{
		delete ButtonVertexBuffer;
	}
	std::vector<Vertex> Vertices;
	Vertex vertex1;
	vertex1.Position.x = 0;
	vertex1.Position.y = 0;
	vertex1.Position.z = 0;
	vertex1.U = 1;
	vertex1.V = 0;
	Vertices.push_back(vertex1);

	Vertex vertex2;
	vertex2.Position.x = 1;
	vertex2.Position.y = 0;
	vertex2.Position.z = 0;
	vertex2.U = 0;
	vertex2.V = 0;
	Vertices.push_back(vertex2);

	Vertex vertex3;
	vertex3.Position.x = 0;
	vertex3.Position.y = 1;
	vertex3.Position.z = 0;
	vertex3.U = 1;
	vertex3.V = 1;
	Vertices.push_back(vertex3);

	Vertex vertex4;
	vertex4.Position.x = 1;
	vertex4.Position.y = 1;
	vertex4.Position.z = 0;
	vertex4.U = 0;
	vertex4.V = 1;
	Vertices.push_back(vertex4);
	ButtonVertexBuffer = new VertexBuffer(Vertices.data(), Vertices.size());
}

void UITextField::Tick()
{
	if (!IsVisible) return;
	TextObject->WrapDistance = std::max(std::max(Size.X * 1.3f, GetMinSize().X), 0.1f);

	ButtonColorMultiplier = 1.f;
	Vector2 Offset;
	if (CurrentScrollObject != nullptr)
	{
		Offset.Y = CurrentScrollObject->Percentage;
	}
	if (Maths::IsPointIn2DBox(OffsetPosition + Offset, OffsetPosition + Size + Offset, Input::MouseLocation))
	{
		if (!IsHovered)
		{
			RedrawUI();
		}
		IsHovered = true;
		ButtonColorMultiplier = 0.8f;
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
			IsEdited = true;
			TextInput::PollForText = true;
			TextInput::Text = EnteredText;
			IsPressed = false;
			TextInput::TextIndex = TextInput::Text.size();
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
	if ((TextInput::TextIndex == TextInput::Text.size() || TextInput::Text.size() == 0) && fmod(Stats::Time, 1) < 0.5f && IsEdited)
	{
		if (TextInput::Text.size() == 0)
		{
			TextInput::TextIndex = 0;
		}
		RendererdText.append("_");
	}
	else if (fmod(Stats::Time, 1) < 0.5f && IsEdited)
	{
		RendererdText.at(TextInput::TextIndex) = '|';
	}
	TextObject->SetColor(EnteredText.empty() && !IsEdited ? TextColor * Vector3(0.75) : TextColor);
	TextObject->SetText(EnteredText.empty() && !IsEdited ? HintText : (IsEdited ? RendererdText : EnteredText));
}

Vector3 UITextField::GetColor()
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

Vector3 UITextField::GetTextColor()
{
	return TextColor;
}

void UITextField::Edit()
{
	IsEdited = true;
	TextInput::PollForText = true;
	TextInput::Text = EnteredText;
	IsPressed = false;
	TextInput::TextIndex = TextInput::Text.size();
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

float UITextField::GetTextSize()
{
	return TextSize;
}

std::string UITextField::GetText()
{
	return EnteredText;
}

bool UITextField::GetIsHovered()
{
	return IsHovered;
}

bool UITextField::GetIsPressed()
{
	return IsPressed;
}

UITextField::UITextField(bool Horizontal, Vector2 Position, Vector3 Color, UICanvas* UI, int ButtonIndex, TextRenderer* Renderer, Shader* ButtonShader)
	: UIBox(Horizontal, Position)
{
	this->ButtonIndex = ButtonIndex;
	this->ButtonShader = ButtonShader;
	this->ParentUI = UI;
	this->Color = Color;
	TextObject = new UIText(0, Vector3(1), HintText, Renderer);
	TextObject->SetTextSize(0.5);
	TextObject->SetPadding(0.005);
	SetHorizontal(false);
	this->Align = UIBox::E_REVERSE;
	
	//TextObject->SetTryFill(true);
	TextObject->Wrap = true;
	AddChild(TextObject);
	MakeGLBuffers();
	//SetMinSize(Vector2(0.1, 0.04));
}

UITextField::~UITextField()
{
	IsEdited = false;
	EnteredText = TextInput::Text;
	TextInput::PollForText = false;
	delete ButtonVertexBuffer;
}

void UITextField::Draw()
{
	ButtonShader->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	ButtonVertexBuffer->Bind();
	ScrollTick(ButtonShader);
	glUniform4f(glGetUniformLocation(ButtonShader->GetShaderID(), "u_transform"), OffsetPosition.X, OffsetPosition.Y, Size.X, Size.Y);
	glUniform4f(glGetUniformLocation(ButtonShader->GetShaderID(), "u_color"),
		ButtonColorMultiplier * Color.X, ButtonColorMultiplier * Color.Y, ButtonColorMultiplier * Color.Z, 1.f);
	glUniform1f(glGetUniformLocation(ButtonShader->GetShaderID(), "u_opacity"), 1.0);
	glUniform1i(glGetUniformLocation(ButtonShader->GetShaderID(), "u_usetexture"), 0);
	glUniform1i(glGetUniformLocation(ButtonShader->GetShaderID(), "u_borderType"), BorderType);
	glUniform1f(glGetUniformLocation(ButtonShader->GetShaderID(), "u_borderScale"), BorderRadius / 20.0f);
	glUniform1f(glGetUniformLocation(ButtonShader->GetShaderID(), "u_aspectratio"), Graphics::AspectRatio);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	ButtonVertexBuffer->Unbind();
}

void UITextField::Update()
{
	if (!TryFill)
	{
		//TextObject->WrapDistance = std::max(std::max(TextObject->GetUsedSize().X, GetMinSize().X), 0.1f);
		//Vector2 TextDesiredSize = TextObject->GetUsedSize();
		//TextDesiredSize += 0.005;
		//TextDesiredSize = TextDesiredSize.Clamp(MinSize, MaxSize);
		//Size = TextDesiredSize;
	}
}