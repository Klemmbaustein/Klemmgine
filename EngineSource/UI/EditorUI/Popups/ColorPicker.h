#if EDITOR
#pragma once
#include <UI/EditorUI/Popups/EditorPopup.h>
#include <Rendering/ShaderManager.h>
#include <UI/UIfwd.h>

class UIVectorField;

class ColorPicker : public EditorPopup
{
	Shader* ColorPickerShaders[2];
	UIBackground* ColorPickerBackgrounds[3];
	float SelectedHue = 0;
	Vector3 SelectedColor;
	void UpdateColors();
	Vector2 SV;
	UIBox* RGBBox;
	UITextField* RGBTexts[3];
	bool DeleteSoon = false;
	void GenerateRGBDisplay();
public:
	UIVectorField* ColorPtr = nullptr;

	ColorPicker(UIVectorField* Color);
	void OnButtonClicked(int Index) override;

	void Tick() override;
	~ColorPicker();
};
#endif