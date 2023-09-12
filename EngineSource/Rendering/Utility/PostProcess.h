#pragma once
#include <string>
#include <vector>

struct Shader;

namespace PostProcess
{
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

	void AddEffect(Effect* NewEffect);
	bool RemoveEffect(Effect*& TargetEffect);

	const std::vector<Effect*> GetCurrentEffects();
}