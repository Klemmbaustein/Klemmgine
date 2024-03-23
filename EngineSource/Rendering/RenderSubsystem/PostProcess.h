#pragma once
#include <string>
#include <vector>
#include "RenderSubsystem.h"

struct Shader;

class PostProcess : public RenderSubsystem
{
public:
	PostProcess();

	static PostProcess* PostProcessSystem;

	enum class EffectType
	{
		World,
		UI,
		World_Internal,
		UI_Internal
	};

	struct Effect
	{
		Effect(std::string FragmentShader, EffectType UsedType);
		void UpdateSize();
		EffectType UsedType = EffectType::World;
		Shader* EffectShader = nullptr;
		unsigned int EffectBuffer = 0;
		unsigned int EffectTexture = 0;
		unsigned int Render(unsigned int TargetBuffer) const;
	};

	static void AddEffect(Effect* NewEffect);
	static bool RemoveEffect(Effect*& TargetEffect);

	static const std::vector<Effect*> GetCurrentEffects();

	void Update() override;

	void Draw();

	bool RenderPostProcess = true;

private:

	unsigned int BloomBuffer = 0, AOBuffer = 0;
	unsigned int MainPostProcessBuffer = 0;
	Shader* PostProcessShader = nullptr;
	Effect* UIMergeEffect = nullptr;
	Effect* AntiAliasEffect = nullptr;

	static std::vector<Effect*> AllEffects;
};