#if !SERVER
#pragma once
#include <UI/Default/TextRenderer.h>
#include <UI/UIBox.h>

struct Shader;
class DrawableText;
class TextRenderer;

/**
 * @brief
 * UI element that displays a Text string with the font from the given TextRenderer.
 *
 *
 * @ingroup UI
 */
class UIText : public UIBox
{
	TextRenderer* Renderer = nullptr;
	Vector3 Color;
	ColoredText RenderedText;
	float TextSize = 0.5f;
	DrawableText* Text = nullptr;
	float TextWidthOverride = 0;
	float Opacity = 1.f;
	SizeMode WrapSizeMode = SizeMode::ScreenRelative;
	float GetRenderedSize() const;
	float GetWrapDistance() const;
public:
	SizeMode TextSizeMode = SizeMode::AspectRelative;

	void Tick() override;
	bool Wrap = false;
	float WrapDistance = 0.0f;
	Vector3 GetColor() const;
	UIText* SetColor(Vector3 NewColor);
	UIText* SetOpacity(float NewOpacity);
	UIText* SetTextSize(float Size);
	float GetTextSize() const;
	UIText* SetTextWidthOverride(float NewTextWidthOverride);
	UIText* SetWrapEnabled(bool WrapEnabled, float WrapDistance, SizeMode WrapSizeMode);

	/**
	 * @brief
	 * Sets the size mode of the text.
	 *
	 * > Note: This has to be used instead of UIBox::SetSizeMode()
	 */
	UIText* SetTextSizeMode(SizeMode NewMode);

	size_t GetNearestLetterAtLocation(Vector2 Position);
	Vector2 GetLetterLocation(size_t Index);

	virtual std::string GetAsString() override;

	UIText* SetText(ColoredText NewText);
	UIText* SetText(std::string NewText);
	std::string GetText() const;
	UIText(float Scale, Vector3 Color, std::string Text, TextRenderer* Renderer);
	UIText(float Scale, ColoredText Text, TextRenderer* Renderer);
	virtual ~UIText();
	void Draw() override;
	void Update() override;
	void OnAttached() override;
	Vector2 GetUsedSize() override;
};
#endif