#if !SERVER
#pragma once
#include <UI/UIBox.h>
#include <Rendering/Graphics.h>

struct Shader;

struct VertexBuffer;
/**
 * @brief
 * UI element that draws a square over the space it occupies.
 *
 * The UIBackground class can have a color, opacity and texture.
 *
 * @ingroup UI
 */
class UIBackground : public UIBox
{
	Shader* BackgroundShader = nullptr;
	void ScrollTick(Shader* UsedShader);
	void MakeGLBuffers(bool InvertTextureCoordinates = false);
	// 0 = none, 1 = texture, 2 texture that should be unloaded with the box
	uint8_t TextureMode = 0;
	unsigned int TextureID = 0;
protected:
	Vector3 Color;
	VertexBuffer* BoxVertexBuffer = nullptr;
	Vector3 ColorMultiplier = 1;
	float Opacity = 1;
	virtual void DrawBackground();
public:
	UIBackground* SetOpacity(float NewOpacity);
	float GetOpacity() const;
	void SetColor(Vector3 NewColor);
	Vector3 GetColor() const;
	bool GetUseTexture() const;
	void SetInvertTextureCoordinates(bool Invert);
	UIBackground* SetUseTexture(bool UseTexture, unsigned int TextureID = 0);
	UIBackground* SetUseTexture(bool UseTexture, std::string TextureName);
	UIBackground(Orientation BoxOrientation, Vector2 Position, Vector3 Color, Vector2 MinScale = Vector2(0), Shader* UsedShader = Graphics::UIShader);
	virtual ~UIBackground();
	void Draw() override;

	void Update() override;
	void OnAttached() override;
};
#endif