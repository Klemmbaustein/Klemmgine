#if EDITOR
#pragma once
#include <UI/EditorUI/Tabs/EditorTab.h>
#include <Rendering/Framebuffer.h>
#include <Rendering/Particle.h>
#include <Rendering/Graphics.h>
#include <Rendering/ShaderManager.h>
#include <Math/Math.h>

class TextRenderer;
class UIText;

class ParticleEditorTab : public EditorTab
{
	std::vector<std::string> ElementMaterials = 
	{
	};

	struct ParticleParam
	{
		void* ValuePointer;
		NativeType::NativeType ParamType;
		std::string Name;

		ParticleParam(void* ValuePointer, NativeType::NativeType ParamType, std::string Name);
	};
	UIBackground* PreviewBackground = nullptr;
	UIScrollBox* ChildBox = nullptr;
	UIScrollBox* SideBar = nullptr;
	std::vector<std::vector<ParticleParam>> Parameters;
	std::vector<std::vector<UIBox*>> ParameterButtons;
	FramebufferObject* PreviewBuffer = nullptr;
	Camera* PreviewCamera = nullptr;

	void GenerateElementButtons(const std::vector<ParticleParam>& ElementParams, UIBox* Target, int ElementIndex);
	void AddParametersForElement(Particles::ParticleElement* Element, std::string* MaterialPtr);
	UIBackground* ParticleViewport = nullptr;
	FramebufferObject* ParticleFramebufferObject = nullptr;
	Particles::ParticleEmitter* Particle;
	std::string LoadedFile;
	float ReActivateDelay = 1.f;
public:
	void OnResized() override;
	ParticleEditorTab(EditorPanel* Parent, std::string File);
	void Tick() override;
	void Load(std::string File) override;
	void Save() override;
	void Generate();
	void OnButtonClicked(int Index) override;
	virtual ~ParticleEditorTab();
};
#endif