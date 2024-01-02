#if !SERVER
#include "UIText.h"
#include <Engine/Log.h>
#include <iostream>
#include <Rendering/Graphics.h>

void UIText::Tick()
{
	Vector2 NewMin = Renderer->GetTextSize(RenderedText, TextSize * 2, Wrap, WrapDistance);
	if (TextWidthOverride > 0)
	{
		NewMin.X = std::max(MinSize.X, TextWidthOverride);
	}
	SetMinSize(NewMin);
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
		RedrawUI();
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

void UIText::SetText(std::string NewText)
{
	SetText({ TextSegment(NewText, Color) });
}

void UIText::SetText(ColoredText NewText)
{
	if (NewText != RenderedText)
	{
		RenderedText = NewText;
		if (Wrap)
		{
			float Distance = WrapDistance;
			if (WrapSizeMode == SizeMode::PixelRelative)
			{
				Distance /= Graphics::AspectRatio;
			}
			Vector2 s = Renderer->GetTextSize(RenderedText, TextSize * 2, Wrap, Distance)
				/ (15.0f * 60.0f);

			if (s.X < Distance)
			{
				Update();
				RedrawUI();
				return;
			}
		}
		InvalidateLayout();
	}
}

size_t UIText::GetNearestLetterAtLocation(Vector2 Location)
{
	size_t Depth = Renderer->GetCharacterIndexADistance(RenderedText, Location.X - OffsetPosition.X, TextSize * 2);
	return Depth;
}

std::string UIText::GetText() const
{
	return TextSegment::CombineToString(RenderedText);
}

UIText::UIText(float Scale, Vector3 Color, std::string Text, TextRenderer* Renderer) : UIBox(UIBox::Orientation::Horizontal, Position)
{
	this->TextSize = Scale;
	this->Color = Color;
	this->Renderer = Renderer;
	RenderedText = { TextSegment(Text, Color) };
}

UIText::UIText(float Scale, ColoredText Text, TextRenderer* Renderer) : UIBox(UIBox::Orientation::Horizontal, Position)
{
	this->TextSize = Scale;
	this->Color = Color;
	this->Renderer = Renderer;
	RenderedText = Text;
}

UIText::~UIText()
{
	if (Text) delete Text;
}

Vector2 UIText::GetLetterLocation(size_t Index)
{
	float Distance = WrapDistance;
	if (WrapSizeMode == SizeMode::PixelRelative)
	{
		Distance /= Graphics::AspectRatio;
	}
	return Renderer->GetLetterPosition(
		RenderedText,
		Index,
		TextSize * 2,
		Wrap,
		WrapDistance
	) + OffsetPosition;
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
	if (Text) delete Text;
	if (Wrap)
	{
		float Distance = WrapDistance;
		if (WrapSizeMode == SizeMode::PixelRelative)
		{
			Distance /= Graphics::AspectRatio;
		}
		Text = Renderer->MakeText(RenderedText, OffsetPosition + Vector2(0, Size.Y - TextSize / 20),
			TextSize * 2, Color, Opacity, Distance);
	}
	else
	{
		Text = Renderer->MakeText(RenderedText, OffsetPosition + Vector2(0, Size.Y - TextSize / 20),
			TextSize * 2, Color, Opacity, 999);
	}
}

void UIText::OnAttached()
{
}

Vector2 UIText::GetUsedSize()
{
	float Distance = WrapDistance;
	if (WrapSizeMode == SizeMode::PixelRelative)
	{
		Distance /= Graphics::AspectRatio;
	}
	return Renderer->GetTextSize(RenderedText, TextSize * 2, Wrap, WrapDistance);
}
#endif