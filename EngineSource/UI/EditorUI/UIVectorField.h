#pragma once
#include <UI/UIBox.h>
#include <UI/UIfwd.h>

class UICanvas;

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
	VecType Type = VecType::xyz;
	UIBox* FieldBox = nullptr;
	UIText* ColorText = nullptr;
public:
	void SendNotifyEvent();
	UIVectorField* SetValueType(VecType Type);
	Vector3 GetValue();
	void SetValue(Vector3 NewValue);
	UIVectorField(Vector2 Position, Vector3 StartValue, UICanvas* ParentUI, int Index, TextRenderer* Renderer);
	~UIVectorField();
	void Update() override;
	void OnChildClicked(int Index) override;
};