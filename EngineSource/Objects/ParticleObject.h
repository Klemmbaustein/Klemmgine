#pragma once
#include <Objects/WorldObject.h>
#include <GENERATED/ParticleObject.h>

class ParticleComponent;

/**
* @brief
* An object representing a particle system in a scene.
*
* Path: Classes/Default
*/
class ParticleObject : public WorldObject
{
	ParticleComponent* Particle = nullptr;
public:
	PARTICLEOBJECT_GENERATED("Default");
	void Begin() override;
	void Destroy() override;
	void Update() override;
	void OnPropertySet() override;

	/**
	* @brief Loads the particle with the given name. (without path or extension)
	* 
	* @param ParticleName
	* The name of the particle file.
	*/
	void LoadParticle(std::string ParticleName);
	std::string ParticleName;
};