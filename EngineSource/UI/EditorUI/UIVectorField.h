#pragma once
#include <UI/UIBox.h>
#include <UI/UIfwd.h>

class UICanvas;

/**
* @brief
* Like a UITextField, but for the Vector3 class instead of strings
* 
* This element only exists in the editor, no C# equivalent exists.
* 
* @ingroup UI
* @ingroup Editor
*/
class UIVectorField : public UIBox
{
public:
	enum class VecType
	{
		xyz = 0,
		rgb = 1,
		PitchYawRoll = 2
	};
protected:
	UITextField* TextFields[3];
	UIButton* ColorDisplay = nullptr;
	void UpdateValues();
	Vector3 Value;
	void Generate();
	TextRenderer* Renderer;
	UICanvas* ParentUI;
	int Index;
	VecType NativeType = VecType::xyz;
	UIBox* FieldBox = nullptr;
	UIText* ColorText = nullptr;
	float Size = 0.0f;
public:
	void SendNotifyEvent();
	UIVectorField* SetValueType(VecType NativeType);
	Vector3 GetValue();
	void SetValue(Vector3 NewValue);
	UIVectorField(float Size, Vector3 StartValue, UICanvas* ParentUI, int Index, TextRenderer* Renderer);
	~UIVectorField();
	void Update() override;
	void OnChildClicked(int Index) override;
};