#pragma once
#include <Objects/Components/Component.h>

namespace Particles
{
	class ParticleEmitter;
}

class ParticleComponent : public Component
{
	Particles::ParticleEmitter* Emitter = nullptr;
	Vector3 Position;
	Vector3 Rotation;
public:
	void Begin() override;
	void Tick() override;
	void Destroy() override;
	
	void LoadParticle(std::string Name);
	void SetActive(bool Active);
	bool GetActive();
	bool GetIsFinished();
	void Reset();


	void SetRelativeRotation(Vector3 NewRotation);
	Vector3 GetRelativeRotation();
	void SetRelativePosition(Vector3 NewPos);
	Vector3 GetRelativePosition();
};