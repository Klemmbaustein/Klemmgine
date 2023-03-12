#include "UIText.h"
#include <Engine/Log.h>

Vector3 UIText::GetColor()
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
		GetAbsoluteParent()->InvalidateLayout();
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
		GetAbsoluteParent()->InvalidateLayout();
	}
	return this;
}

float UIText::GetTextSize()
{
	return TextSize;
}

UIText* UIText::SetTextWidthOverride(float NewTextWidthOverride)
{
	if (TextWidthOverride != NewTextWidthOverride)
	{
		TextWidthOverride = NewTextWidthOverride;
		GetAbsoluteParent()->InvalidateLayout();
	}
	return this;
}

void UIText::SetText(std::string NewText)
{
	if (NewText != TextSegment::CombineToString(RenderedText))
	{
		RenderedText = { TextSegment(NewText, Color) };
		if (Wrap)
		{
			Vector2 s = Renderer->GetTextSize(RenderedText, TextSize * 2, Wrap, WrapDistance)
				/ ((30 + Renderer->CharacterSizeInPixels / 2) * 60.f);
			if (s.X < WrapDistance)
			{
				Update();
				RedrawUI();
				return;
			}
		}
		GetAbsoluteParent()->InvalidateLayout();
	}
}

void UIText::SetText(ColoredText NewText)
{
	if (NewText != RenderedText)
	{
		RenderedText = NewText;
		if (Wrap)
		{
			Vector2 s = Renderer->GetTextSize(RenderedText, TextSize * 2, Wrap, WrapDistance)
				/ ((30 + Renderer->CharacterSizeInPixels / 2) * 60.f);
			if (s.X < WrapDistance)
			{
				Update();
				RedrawUI();
				return;
			}
		}
		GetAbsoluteParent()->InvalidateLayout();
	}
}

size_t UIText::GetNearestLetterAtLocation(Vector2 Location, Vector2& LetterOutLocation)
{
	size_t Depth = Renderer->GetCharacterIndexADistance(RenderedText, Location.X - OffsetPosition.X, TextSize * 2, LetterOutLocation);
	LetterOutLocation = LetterOutLocation + OffsetPosition;
	return Depth;
}

std::string UIText::GetText()
{
	return TextSegment::CombineToString(RenderedText);
}

UIText::UIText(float Scale, Vector3 Color, std::string Text, TextRenderer* Renderer) : UIBox(true, Position)
{
	this->TextSize = Scale;
	this->Color = Color;
	this->Renderer = Renderer;
	RenderedText = { TextSegment(Text, Color) };
}

UIText::UIText(float Scale, ColoredText Text, TextRenderer* Renderer) : UIBox(true, Position)
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
	std::string Text = TextSegment::CombineToString(RenderedText);
	return Vector2(Renderer->GetTextSize({ TextSegment(Text.substr(0, Index), 1) }, TextSize * 4, false, 999999).X, 0) + OffsetPosition;
}

void UIText::Draw()
{
	if (IsDynamic)
	{
		Renderer->RenderText(RenderedText, OffsetPosition, TextSize * 2, Color, Opacity, 999, CurrentScrollObject);
	}
	else if (Text)
	{
		Text->Opacity = Opacity;
		Text->Draw(CurrentScrollObject);
	}
}

void UIText::Update()
{
	if (!IsDynamic)
	{
		if (Text) delete Text;
		if (Wrap)
		{
			Text = Renderer->MakeText(RenderedText, OffsetPosition + Vector2(0, Size.Y - 0.025f), TextSize * 2, Color, Opacity, WrapDistance);
		}
		else
		{
			Text = Renderer->MakeText(RenderedText, OffsetPosition + Vector2(0, Size.Y - 0.025f), TextSize * 2, Color, Opacity, 999);
		}
	}
	MinSize = Renderer->GetTextSize(RenderedText, TextSize * 2, Wrap, WrapDistance)
		/ ((30 + Renderer->CharacterSizeInPixels / 2) / 60.f);
	MinSize.X = std::max(MinSize.X, TextWidthOverride);
}

void UIText::OnAttached()
{
}

Vector2 UIText::GetUsedSize()
{
	return Renderer->GetTextSize(RenderedText, TextSize * 2, Wrap, WrapDistance);
}
