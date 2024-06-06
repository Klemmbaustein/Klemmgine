#pragma once
#include <UI/UIBackground.h>
#include <UI/UICanvas.h>

class TextRenderer;
class UIText;
struct VertexBuffer;
class UIBackground;

/**
 * @brief
 * Functions like a UIButton, but you can input text into it.
 *
 * The Text field can have a color and opacity.
 *
 * @ingroup UI
 */
class UITextField : public UIBackground
{
	bool IsHovered = false;
	bool IsPressed = false;
	Vector2 IBeamPosition;
	Vector2 IBeamScale = Vector2(0.001f, 0.03f);
	bool ShowIBeam = false;
	Vector3 TextColor = 1;
	UICanvas* ParentUI;
	UIText* TextObject = nullptr;
	int ButtonIndex;
	bool IsEdited = false;
	std::string EnteredText = "";
	float TextSize = 0.5f;
	void Tick() override;
	float TextFieldTimer = 0;
	float DoubleClickTimer = 0;
	bool Dragging = false;

	Vector2 TextHighlightPos;
	Vector2 TextHighlightSize;
public:
	Vector3 GetColor() const;
	UITextField* SetColor(Vector3 NewColor);
	UITextField* SetTextColor(Vector3 NewColor);
	Vector3 GetTextColor() const;
	UIBox* ParentOverride = nullptr;
	/**
	* @brief
	* Takes keyboard focus to let the user input text.
	* 
	* This functions the same as clicking on the field.
	*/
	void Edit();
	bool GetIsEdited() const { return IsEdited; }
	UITextField* SetText(std::string NewText);
	UITextField* SetTextSize(float NewTextSize);
	float GetTextSize() const;
	
	/**
	* @brief
	* Gets the text currently typed into the text field.
	*/
	std::string GetText();
	/// Will be displayed when the text field is empty
	std::string HintText;
	bool GetIsHovered() const;
	bool GetIsPressed() const;

	UITextField(Vector2 Position, Vector3 Color, UICanvas* UI, int ButtonIndex, TextRenderer* Renderer, Shader* ButtonShader = Graphics::UIShader);
	~UITextField() override;
	void DrawBackground() override;
};
