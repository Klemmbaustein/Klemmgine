#pragma once
#include <Objects/WorldObject.h>
#include <Sound/Sound.h>
#include <GENERATED/GENERATED_SoundObject.h>
class SoundObject : public WorldObject
{
public:
	SOUNDOBJECT_GENERATED("Default/Misc");
	virtual void Begin() override;
	virtual void Tick() override;
	virtual void OnPropertySet() override;
	virtual void Destroy() override;
	void LoadSound(std::string SoundName);
protected:
	bool IsSpacialSound = true;
	bool IsLooping = true;
	std::string Filename;
	float Pitch = 1, Volume = 1, FalloffRange = 10;
	Sound::SoundBuffer* Buffer = nullptr;
	Sound::SoundSource Source;
};