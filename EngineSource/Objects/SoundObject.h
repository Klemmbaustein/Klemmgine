#pragma once
#include <Objects/SceneObject.h>
#include <Engine/Subsystem/Sound.h>
#include <GENERATED/SoundObject.h>

/**
* @brief
* An object representing a 3d sound in a scene.
*
* Path: Classes/Default
* 
* @ingroup Objects
*/
class SoundObject : public SceneObject
{
public:
	SOUNDOBJECT_GENERATED("Default");
	virtual void Begin() override;
	virtual void Update() override;
	virtual void OnPropertySet() override;
	virtual void Destroy() override;

	/**
	* @brief Loads the sound with the given name. (without path or extension)
	* 
	* @param SoundName
	* The name of the sound file.
	*/
	void LoadSound(std::string SoundName);
	/**
	* @brief
	* An editor parameter. True if the sound is 3d. False if not.
	*/
	bool IsSpatialSound = true;

	/**
	* An editor parameter. True if the sound should repeat once it is finished. False if not.
	*/
	bool IsLooping = true;
	std::string Filename;
	/**
	* An editor parameter. The pitch of the sound.
	*/
	float Pitch = 1;

	/**
	* An editor parameter. The volume of the sound.
	*/
	float Volume = 1;

	/**
	* An editor parameter. The falloff of the sound.
	*/
	float FalloffRange = 10;
protected:
	Sound::SoundBuffer* Buffer = nullptr;
	Sound::SoundSource Source;
};