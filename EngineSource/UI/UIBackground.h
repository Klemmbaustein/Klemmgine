#pragma once
#include <UI/UIBox.h>
#include <World/Graphics.h>

struct Shader;

struct VertexBuffer;

class UIBackground : public UIBox
{
	VertexBuffer* BoxVertexBuffer = nullptr;
	Vector3 Color;
	Shader* BackgroundShader;
	void ScrollTick(Shader* UsedShader);
	void MakeGLBuffers(bool InvertTextureCoordinates = false);
	bool UseTexture = false;
	unsigned int TextureID = 0;
	float Opacity = 1;
public:
	UIBackground* SetOpacity(float NewOpacity);
	float GetOpacity();
	void SetColor(Vector3 NewColor);
	Vector3 GetColor();
	void SetInvertTextureCoordinates(bool Invert);
	void SetUseTexture(bool UseTexture, unsigned int TextureID = 0);
	UIBackground(bool Horizontal, Vector2 Position, Vector3 Color, Vector2 MinScale = Vector2(0), Shader* UsedShader = Graphics::UIShader);
	virtual ~UIBackground();
	void Draw() override;
	void Update() override;
	void OnAttached() override;
};