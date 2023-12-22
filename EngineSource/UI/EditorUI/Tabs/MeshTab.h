#pragma once
#include "EditorTab.h"
#include <UI/UITextField.h>
#include <Rendering/Mesh/ModelGenerator.h>

class FramebufferObject;
class Model;
class UIText;
class UIScrollBox;

class MeshTab : public EditorTab
{
public:
	UIBackground* PreviewWindow = nullptr;

	MeshTab(Vector3* UIColors, TextRenderer* Renderer);
	void Tick() override;
	void Load(std::string File) override;
	void ReloadMesh();
	void Save() override;
	void Generate();
	void OnButtonClicked(int Index) override;
	virtual ~MeshTab();
protected:
	void UpdatePreviewModel();
	FramebufferObject* PreviewBuffer = nullptr;
	Model* PreviewModel = nullptr;
	ModelGenerator::ModelData ModelData;
	bool CastShadow = false;
	bool HasCollision = true;
	bool TwoSided = true;
	std::string MeshPath;

	UIText* TabName = nullptr;;
	TextRenderer* Renderer;
	UIBox* Rows[2];
	std::vector<UITextField*> MaterialTextFields;
	std::string Filepath;
	std::string InitialName;
	std::string Options[3] =
	{
		"Cast shadow   :",
		"Has collision :",
		"Two sided     :"
	};
	bool* OptionVariables[3] =
	{
		&CastShadow,
		&HasCollision,
		&TwoSided
	};
	Transform CameraTransform;
};