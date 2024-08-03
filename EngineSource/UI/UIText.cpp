#if !SERVER
#include "UIText.h"
#include <Engine/Log.h>
#include <iostream>
#include <Rendering/Graphics.h>
#include <Engine/Utility/StringUtility.h>

float UIText::GetRenderedSize() const
{
	float RenderedSize = TextSize;
	if (TextSizeMode == SizeMode::PixelRelative)
	{
		RenderedSize = RenderedSize / Graphics::WindowResolution.Y * 50 * UIBox::DpiScale;
	}
	return RenderedSize;
}

float UIText::GetWrapDistance() const
{
	float Distance = WrapDistance * 2;
	if (WrapSizeMode == SizeMode::AspectRelative)
	{
		Distance /= Graphics::AspectRatio;
	}
	if (WrapSizeMode == SizeMode::PixelRelative)
	{
		Distance = UIBox::PixelSizeToScreenSize(Vector2(Distance, 0.0)).X;
	}
	return Distance;
}

void UIText::Tick()
{
	SetMinSize(GetUsedSize());
}

Vector3 UIText::GetColor() const
{
	return Color;
}

UIText* UIText::SetColor(Vector3 NewColor)
{
	if (Color != NewColor)
	{
		Color = NewColor;
		for (auto& i : RenderedText)
		{
			i.Color = Color;
		}
		InvalidateLayout();
	}
	return this;
}

UIText* UIText::SetOpacity(float NewOpacity)
{
	if (Opacity != NewOpacity)
	{
		Opacity = NewOpacity;
		RedrawElement();
	}
	return this;
}

UIText* UIText::SetTextSize(float Size)
{
	if (Size != TextSize)
	{
		TextSize = Size;
		InvalidateLayout();
	}
	return this;
}


float UIText::GetTextSize() const
{
	return TextSize;
}

UIText* UIText::SetTextWidthOverride(float NewTextWidthOverride)
{
	if (TextWidthOverride != NewTextWidthOverride)
	{
		TextWidthOverride = NewTextWidthOverride;
		InvalidateLayout();
	}
	return this;
}

UIText* UIText::SetWrapEnabled(bool WrapEnabled, float WrapDistance, SizeMode WrapSizeMode)
{
	this->Wrap = WrapEnabled;
	this->WrapDistance = WrapDistance;
	this->WrapSizeMode = WrapSizeMode;
	return this;
}

UIText* UIText::SetText(std::string NewText)
{
	return SetText({ TextSegment(NewText, Color) });
}

UIText* UIText::SetText(ColoredText NewText)
{
	if (NewText != RenderedText)
	{
		RenderedText = NewText;
		if (Wrap)
		{
			if (!Renderer)
				return this;

			Vector2 s = Renderer->GetTextSize(RenderedText, GetRenderedSize(), Wrap, GetWrapDistance());
		}
		InvalidateLayout();
		RedrawElement();
	}
	return this;
}

UIText* UIText::SetTextSizeMode(SizeMode NewMode)
{
	if (this->TextSizeMode != NewMode)
	{
		RedrawElement();
		this->TextSizeMode = NewMode;
	}
	return this;
}

size_t UIText::GetNearestLetterAtLocation(Vector2 Position)
{
	size_t Depth = Renderer->GetCharacterIndexADistance(RenderedText, Position.X - OffsetPosition.X, TextSize);
	return Depth;
}

std::string UIText::GetText() const
{
	return TextSegment::CombineToString(RenderedText);
}

UIText::UIText(float Scale, Vector3 Color, std::string Text, TextRenderer* Renderer) : UIBox(UIBox::Orientation::Horizontal, 0)
{
	this->TextSize = Scale;
	this->Color = Color;
	this->Renderer = Renderer;
	RenderedText = { TextSegment(Text, Color) };
}

UIText::UIText(float Scale, ColoredText Text, TextRenderer* Renderer) : UIBox(UIBox::Orientation::Horizontal, 0)
{
	this->TextSize = Scale;
	this->Renderer = Renderer;
	RenderedText = Text;
}

UIText::~UIText()
{
	if (Text) delete Text;
}

Vector2 UIText::GetLetterLocation(size_t Index)
{
	if (!Renderer) return 0;
	std::string Text = TextSegment::CombineToString(RenderedText);
	return Vector2(Renderer->GetTextSize({ TextSegment(Text.substr(0, Index), 1) }, GetRenderedSize(), false, 999).X, 0) + OffsetPosition;
}

std::string UIText::GetAsString()
{
	return "UIText '" + TextSegment::CombineToString(RenderedText) + "'";
}

void UIText::Draw()
{
	if (Text)
	{
		Text->Opacity = Opacity;
		Text->Draw(CurrentScrollObject);
	}
}

void UIText::Update()
{
	if (!Renderer)
	{
		return;
	}
	if (Text)
	{
		delete Text;
	}
	if (Wrap)
	{
		Text = Renderer->MakeText(RenderedText, OffsetPosition + Vector2(0, Size.Y - GetRenderedSize() / 20),
			GetRenderedSize(), Color, Opacity, GetWrapDistance());
	}
	else
	{
		Text = Renderer->MakeText(RenderedText, OffsetPosition + Vector2(0, Size.Y - GetRenderedSize() / 20),
			GetRenderedSize(), Color, Opacity, 999);
	}
}

void UIText::OnAttached()
{
}

Vector2 UIText::GetUsedSize()
{
	if (!Renderer)
		return 0;

	Vector2 Size = Renderer->GetTextSize(RenderedText, GetRenderedSize(), Wrap, GetWrapDistance());

	if (TextWidthOverride != 0)
	{
		return Vector2(TextWidthOverride, Size.Y);
	}
	return Size;
}
#endif