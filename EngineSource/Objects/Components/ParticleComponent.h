#pragma once
#include <Objects/Components/Component.h>

namespace Particles
{
	class ParticleEmitter;
}

class ParticleComponent : public Component
{
	Particles::ParticleEmitter* Emitter = nullptr;
public:
	void Begin() override;
	void Tick() override;
	void Destroy() override;
	
	void LoadParticle(std::string Name);
	void SetActive(bool Active);
	bool GetActive();
	bool GetIsFinished();
	void Reset();
};