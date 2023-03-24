#if 0
#pragma once
#include <UI/EditorUI/EditorTab.h>
#include <UI/UITextField.h>
#include <UI/EditorUI/EditorTab.h>
#include <Rendering/Texture/Material.h>
class Model;
class UIText;
class UIScrollBox;
class MaterialTemplateTab : public EditorTab
{
public:
	virtual void OnButtonClicked(int Index) override;

	MaterialTemplateTab(Vector3* UIColors, TextRenderer* Text, unsigned int XButtonIndex);

	void Tick() override; //Render and Update
	void Load(std::string File) override;
	void Save() override;
	void GenerateUI();
private:
	Material LoadedMaterial;
	UITextField* ShaderTextFields[2];
	std::vector<UIBox*> TextFields;
	UIText* TabName = nullptr;
	UIBox* Rows[2];
	unsigned int XTexture;
	TextRenderer* Renderer;
	std::string Filepath;
};
#endif