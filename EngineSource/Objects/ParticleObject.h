#pragma once
#include <Objects/WorldObject.h>
#include <GENERATED/ParticleObject.h>

class ParticleComponent;

class ParticleObject : public WorldObject
{
	ParticleComponent* Particle = nullptr;
public:
	PARTICLEOBJECT_GENERATED("Default");
	void Begin() override;
	void Destroy() override;
	void Tick() override;
	void OnPropertySet() override;
	void LoadParticle(std::string ParticleName);
	std::string ParticleName;
};