#if !SERVER
#pragma once
#include <UI/UIBox.h>
#include <UI/Default/UICanvas.h>
#include <UI/UIBackground.h>

/**
* @brief
* A button.
* 
* If the Parent UICanvas has been specified, the UIButton will call the OnClicked(int Index)
* function with the given ButtonIndex.
* 
* Buttons function like a UIBackground and can have color, opacity and texture.
*/
class UIButton : public UIBackground
{
protected:
	Vector2 Offset;
	bool IsHovered = false;
	bool IsPressed = false;
	UICanvas* ParentUI;
	int ButtonIndex;
	bool CanBeDragged = false;
	virtual void Tick() override;
	virtual void Update() override;
	virtual void OnClicked();
	bool ClickStartedOnButton = false;
public:
	virtual std::string GetAsString() override;
	UIButton* SetCanBeDragged(bool NewCanBeDragged);
	bool GetIsHovered() const;
	bool GetIsPressed() const;
	int GetIndex() const;

	UIButton(Orientation BoxOrientation, Vector2 Position, Vector3 Color, UICanvas* Parent, int ButtonIndex, Shader* ButtonShader = Graphics::UIShader);
	~UIButton();
	UIBox* ParentOverride = nullptr;
};
#endif