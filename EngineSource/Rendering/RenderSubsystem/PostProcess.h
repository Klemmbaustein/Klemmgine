#pragma once
#include <string>
#include <vector>
#include "RenderSubsystem.h"

struct Shader;

/**
* @brief
* Post processing subsystem.
* 
* @ingroup Subsystem
*/
class PostProcess : public RenderSubsystem
{
public:
	PostProcess();

	/// A pointer to the current post process subsystem.
	static PostProcess* PostProcessSystem;

	/// The type of a post processing effect.
	enum class EffectType
	{
		/// Applied to the 3d scene.
		World,
		/// Applied to the UI.
		UI,
		World_Internal,
		UI_Internal
	};

	/**
	* @brief
	* A post processing effect.
	*/
	struct Effect
	{
		/// Loads an effect with the given shader and type.
		Effect(std::string FragmentShader, EffectType UsedType);
		void UpdateSize();
		EffectType UsedType = EffectType::World;
		Shader* EffectShader = nullptr;
		unsigned int EffectBuffer = 0;
		unsigned int EffectTexture = 0;
		unsigned int Render(unsigned int TargetBuffer) const;
	};

	/**
	* @brief
	* Registers a new post processing effect.
	*/
	static void AddEffect(Effect* NewEffect);
	/**
	* @brief
	* Removes a registered post processing effect.
	* 
	* The Effect will be set to nullptr if the effect has been removed.
	* 
	* @return
	* True if the effect exists, false if not.
	*/
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