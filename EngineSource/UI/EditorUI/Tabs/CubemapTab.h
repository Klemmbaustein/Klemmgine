/**
* @file
* @brief
* Cubemap Editor UI
* 
* @todo Rewrite Cubemap Editor UI.
*/

#if EDITOR && 0
#pragma once
#include <UI/EditorUI/Tabs/EditorTab.h>
#include <UI/UITextField.h>
#include <Engine/File/Save.h>

class FramebufferObject;
class Model;
class UIText;
class UIScrollBox;

class CubemapTab : public EditorTab
{
public:
	UIBackground* PreviewWindow = nullptr;

	CubemapTab(Vector3* UIColors, TextRenderer* Renderer);
	void Tick() override;
	void Load(std::string File) override;
	void Save() override;
	void OnButtonClicked(int Index) override;
	void Generate();
	virtual ~CubemapTab();
	void UpdateLayout() override;
protected:
	std::vector<std::string> DisplayNames = { "Right face", "Left face", "Upper face", "Lower face", "Front face", "Back face" };
	std::vector<std::string> Cubenames = { "right", "left", "up", "down", "front", "back" };

	std::vector<UITextField*> SideFields;
	SaveGame* SaveFile = nullptr;
	void UpdatePreviewModel();
	FramebufferObject* PreviewBuffer = nullptr;
	Model* PreviewModel = nullptr;
	Transform CameraTransform;
	TextRenderer* Renderer;
	UIText* TabName;
	UIBox* CubemapSidesBox = nullptr;
	std::string InitialName;
};
#endif