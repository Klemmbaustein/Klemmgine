#if EDITOR
#pragma once
#include <UI/UITextField.h>
#include <UI/EditorUI/Tabs/EditorTab.h>
#include <UI/EditorUI/UIVectorField.h>
#include <Rendering/Texture/Material.h>

class FramebufferObject;
class Model;
class Camera;

class MaterialTab : public EditorTab
{
public:
	virtual void OnButtonClicked(int Index) override;

	MaterialTab(Vector3* UIColors, TextRenderer* Text);


	void Tick() override; //Render and Update
	void Load(std::string File) override;
	void UpdateLayout() override;
	void Save();
	void GenerateUI();
	void GenerateMaterialProperties();
	void UpdateModel();
private:
	int RedrawFrames = 0;
	UIBackground* PreviewWindow = nullptr;
	FramebufferObject* PreviewBuffer = nullptr;
	Model* PreviewModel = nullptr;
	Camera* PreviewCamera;

	std::vector<unsigned int> PreviewTextures;
	UITextField* ShaderTextFields[2];
	std::vector<UIBox*> TextFields;
	std::vector<UIBox*> TextureDropdowns;
	UIText* TabName = nullptr;
	UIBox* Rows[2];
	TextRenderer* Renderer;
	std::string Filepath;

	Material LoadedMaterial;

	UIText* CutoutText = nullptr, *TranslucencyText = nullptr;
	UIText* ShaderTexts[2];
};
#endif