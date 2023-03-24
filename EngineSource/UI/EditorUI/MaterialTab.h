#if 0
#pragma once
#include <UI/UITextField.h>
#include <UI/EditorUI/EditorTab.h>
#include <UI/EditorUI/UIVectorField.h>
#include "MaterialTemplateTab.h"

class MaterialTab : public EditorTab
{
public:
	virtual void OnButtonClicked(int Index) override;

	MaterialTab(Vector3* UIColors, TextRenderer* Text, unsigned int ReloadTexture);


	void Tick() override; //Render and Update
	void Load(std::string File) override;
	void LoadTemplate(std::string Template);
	void FetchTemplate(std::string Template);
	void Save();
	void GenerateUI();
private:
	UITextField* ShaderTextFields[2];
	std::vector<UIBox*> TextFields;
	UIText* TabName = nullptr;
	UIBox* Rows[2];
	unsigned int ReloadTexture = 0;
	TextRenderer* Renderer;
	std::string Filepath;

	Material LoadedMaterial;

	UITextField* ParentTemplateText;
	UIBox* RightRow;
	UIText* CutoutText = nullptr, *TranslucencyText = nullptr;
	UIText* ShaderTexts[2];
};
#endif