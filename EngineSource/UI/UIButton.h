#pragma once
#include <UI/UIBox.h>
#include <UI/Default/UICanvas.h>
#include <Rendering/Graphics.h>

struct VertexBuffer;

class UIButton : public UIBox
{
	float ButtonColorMultiplier = 1.f;
	Vector2 Offset;
	VertexBuffer* ButtonVertexBuffer;
	bool IsHovered = false;
	bool IsPressed = false;
	Vector3 Color = Vector3(0.5);
	Shader* ButtonShader = nullptr;
	UICanvas* ParentUI;
	int ButtonIndex;
	bool IsSelected = false;
	bool NeedsToBeSelected = false;
	bool CanBeDragged = false;
	float Opacity = 1;
	bool UseTexture = false;
	unsigned int TextureID = 0;
	void ScrollTick(Shader* UsedShader);
	void MakeGLBuffers();
	void Tick() override;
public:
	bool GetUseTexture();
	UIButton* SetOpacity(float NewOpacity);
	float GetOpacity();
	void SetCanBeDragged(bool NewCanBeDragged);
	bool GetIsSelected();
	void SetNeedsToBeSelected(bool NeedsToBeSelected);
	bool GetIsHovered();
	bool GetIsPressed();
	int GetIndex();
	UIButton* SetColor(Vector3 NewColor);
	Vector3 GetColor();
	UIButton* SetUseTexture(bool UseTexture, unsigned int TextureID = 0);

	UIButton(bool Horizontal, Vector2 Position, Vector3 Color, UICanvas* UI, int ButtonIndex, Shader* ButtonShader = Graphics::UIShader);
	~UIButton();

	void Update() override;
	void Draw() override;
	UIBox* ParentOverride = nullptr;
};