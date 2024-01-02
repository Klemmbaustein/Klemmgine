#if EDITOR
#pragma once
#include <UI/EditorUI/Tabs/EditorTab.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Rendering/Particle.h>
#include <Rendering/Graphics.h>
#include <Rendering/Utility/ShaderManager.h>
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
		Type::TypeEnum ParamType;
		std::string Name;

		ParticleParam(void* ValuePointer, Type::TypeEnum ParamType, std::string Name);
	};
	UIScrollBox* ChildBox = nullptr;
	std::vector<std::vector<ParticleParam>> Parameters;
	std::vector<std::vector<UIBox*>> ParameterButtons;

	void GenerateElementButtons(const std::vector<ParticleParam>& ElementParams, UIBox* Target);
	void AddParametersForElement(Particles::ParticleElement* Element, std::string* MaterialPtr);
	UIBackground* ParticleViewport = nullptr;
	FramebufferObject* ParticleFramebufferObject = nullptr;
	Particles::ParticleEmitter* Particle;
	std::string CurrentSystemFile;
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