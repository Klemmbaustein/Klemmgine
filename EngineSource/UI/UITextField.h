#pragma once
#include <UI/UIBox.h>
#include <UI/Default/UICanvas.h>
#include <World/Graphics.h>

class TextRenderer;
class UIText;
struct VertexBuffer;
class UIBackground;

class UITextField : public UIBox
{
	VertexBuffer* ButtonVertexBuffer = nullptr;
	bool IsHovered = false;
	bool IsPressed = false;
	Vector2 IBeamPosition;
	Vector2 IBeamScale = Vector2(0.001, 0.03);
	bool ShowIBeam = false;
	Vector3 Color = Vector3(0.5);
	Vector3 TextColor = 1;
	Shader* ButtonShader = nullptr;
	UICanvas* ParentUI;
	UIText* TextObject = nullptr;
	int ButtonIndex;
	bool IsEdited = false;
	std::string EnteredText = "";
	float TextSize = 0.5f;
	void ScrollTick(Shader* UsedShader);
	void MakeGLBuffers();
	void Tick() override;
	float ButtonColorMultiplier = 1;
public:
	Vector3 GetColor();
	UITextField* SetColor(Vector3 NewColor);
	UITextField* SetTextColor(Vector3 NewColor);
	Vector3 GetTextColor();
	UIBox* ParentOverride = nullptr;
	void Edit();
	bool GetIsEdited() { return IsEdited; }
	UITextField* SetText(std::string NewText);
	UITextField* SetTextSize(float NewTextSize);
	float GetTextSize();
	std::string GetText();
	std::string HintText; // Will be displayed when the text field is empty
	bool GetIsHovered();
	bool GetIsPressed();

	UITextField(bool Horizontal, Vector2 Position, Vector3 Color, UICanvas* UI, int ButtonIndex, TextRenderer* Renderer, Shader* ButtonShader = Graphics::UIShader);
	~UITextField() override;
	void Draw() override;
	void Update() override;
};
