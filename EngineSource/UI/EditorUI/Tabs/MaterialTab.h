#if EDITOR
#pragma once
#include <UI/UITextField.h>
#include <UI/EditorUI/Tabs/EditorTab.h>
#include <UI/EditorUI/UIVectorField.h>
#include <Rendering/Texture/Material.h>

class FramebufferObject;
class Model;
class Camera;

/**
* @brief
* Tab for editing material (.jsmat) files.
* 
* @ingroup Editor
*/
class MaterialTab : public EditorTab
{
public:
	virtual void OnButtonClicked(int Index) override;

	MaterialTab(EditorPanel* Parent, std::string File);
	~MaterialTab() override;

	void Tick() override; //Render and Update
	void Load(std::string File) override;
	void OnResized() override;
	void Save() override;
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
	UIBox* Rows[2];
	std::string Filepath;

	Material LoadedMaterial;

	UIText* CutoutText = nullptr, *TranslucencyText = nullptr;
	UIText* ShaderTexts[2];
};
#endif