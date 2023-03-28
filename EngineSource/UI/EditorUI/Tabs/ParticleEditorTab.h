#if 0
#pragma once
#include <UI/EditorUI/EditorPanel.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Rendering/Particle.h>
#include <World/Graphics.h>
#include <Rendering/Utility/ShaderManager.h>
#include <Math/Math.h>

class TextRenderer;
class UIText;

class ParticleEditorPanel : public EditorPanel
{
	std::vector<std::string> ElementMaterials = 
	{
	};
	const std::vector <std::string> ElementParametersColumn1 =
	{
		"Direction",
		"Direction Random",
		"Scale",
		"Lifetime",
		"Spawn Delay",
		"Num Loops",
		"Material"
	};
	const std::vector <std::string> ElementParametersColumn2 =
	{
		"Position Range",
		"Force",
		"Start Scale",
		"End Scale"
	}; 
	std::vector<UIBox*> GeneratedUI;
	std::vector<UIBox*> SettingsButtons;

	ScrollObject ElementScrollObject = ScrollObject(Vector2(0.3f, 0.65f), Vector2(0.4, 1.2), 15);
	TextRenderer* TabText = nullptr;
	UIBackground* ParticleViewport = nullptr;
	UIScrollBox* ParticleSettingsScrollBox = nullptr;
	FramebufferObject* ParticleFramebufferObject = nullptr;
	Particles::ParticleEmitter* Particle;
	std::string CurrentSystemFile;
	unsigned int SelectedElement = 0;
	unsigned int RemoveTexture = 0;
	float ReactivateDelay = 1.f;
	UIText* ParticleViewportText = nullptr;
	UIText* SelectedElementText = nullptr;
public:
	ParticleEditorPanel(Vector3* UIColors, TextRenderer* Text, unsigned int RemoveTexture, unsigned int ReloadTexture);
	void Tick() override;
	void Load(std::string File) override;
	void ReloadMesh();
	void Save() override;
	void Generate();
	void OnButtonClicked(int Index) override;
	virtual ~ParticleEditorPanel();
};
#endif