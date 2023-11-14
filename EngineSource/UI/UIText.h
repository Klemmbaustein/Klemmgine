#if !SERVER
#pragma once
#include <UI/Default/TextRenderer.h>
#include <UI/UIBox.h>

struct Shader;
class DrawableText;
class TextRenderer;

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
public:

	void Tick() override;
	bool Wrap = false;
	float WrapDistance = 0.0f;
	Vector3 GetColor();
	UIText* SetColor(Vector3 NewColor);
	UIText* SetOpacity(float NewOpacity);
	UIText* SetTextSize(float Size);
	float GetTextSize();
	UIText* SetTextWidthOverride(float NewTextWidthOverride);
	UIText* SetWrapEnabled(bool WrapEnabled, float WrapDistance, SizeMode WrapSizeMode);

	size_t GetNearestLetterAtLocation(Vector2 Location);
	Vector2 GetLetterLocation(size_t Index);


	void SetText(ColoredText NewText);
	void SetText(std::string NewText);
	std::string GetText();
	UIText(float Scale, Vector3 Color, std::string Text, TextRenderer* Renderer);
	UIText(float Scale, ColoredText Text, TextRenderer* Renderer);
	virtual ~UIText();
	void Draw() override;
	void Update() override;
	void OnAttached() override;
	Vector2 GetUsedSize() override;
};
#endif