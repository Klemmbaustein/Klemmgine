#if !SERVER
#pragma once
#include <UI/UIBox.h>
#include <Rendering/Graphics.h>

struct Shader;

struct VertexBuffer;

class UIBackground : public UIBox
{
	VertexBuffer* BoxVertexBuffer = nullptr;
	Vector3 Color;
	Shader* BackgroundShader = nullptr;
	void ScrollTick(Shader* UsedShader);
	void MakeGLBuffers(bool InvertTextureCoordinates = false);
	// 0 = none, 1 = texture, 2 texture that should be unloaded with the box
	uint8_t TextureMode = 0;
	unsigned int TextureID = 0;
	float Opacity = 1;
public:
	UIBackground* SetOpacity(float NewOpacity);
	float GetOpacity() const;
	void SetColor(Vector3 NewColor);
	Vector3 GetColor() const;
	bool GetUseTexture() const;
	void SetInvertTextureCoordinates(bool Invert);
	UIBackground* SetUseTexture(bool UseTexture, unsigned int TextureID = 0);
	UIBackground* SetUseTexture(bool UseTexture, std::string TextureName);
	UIBackground(bool Horizontal, Vector2 Position, Vector3 Color, Vector2 MinScale = Vector2(0), Shader* UsedShader = Graphics::UIShader);
	virtual ~UIBackground();
	void Draw() override;

	void Update() override;
	void OnAttached() override;
};
#endif