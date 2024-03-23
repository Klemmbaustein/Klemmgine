#pragma once
#include <string>
#include <Math/Vector.h>
#include "Subsystem.h"

class Sound : public Subsystem
{
public:
	static Sound* SoundSystem;
	struct SoundSource
	{
		SoundSource(unsigned int Buffer, float Pitch, float Volume, bool Looping);
		SoundSource()
		{

		}
		void Stop();
		void SetPitch(float NewPitch);
		void SetVolume(float NewVolume);
		float GetPitch();
		float GetVolume();
		bool GetLooping();
	private:
		unsigned int Source = 0;  //OpenAL source ID
		float Pitch = 0; //Sound source pitch
		float Volume = 0; //Sound volume (or 'gain')
		bool Looping = false; //if the sound should looping
	};

	struct SoundBuffer
	{
		SoundBuffer(std::string File);

		SoundBuffer(unsigned int Buffer, std::string Name)
		{
			this->Buffer = Buffer;
			this->Name = Name;
		}
		~SoundBuffer();
		unsigned int Buffer;
		std::string Name;
	};

	float MasterVolume = 1.0f;

	Sound();
	virtual ~Sound() override;

	void StopAllSounds();
	void Update() override;
	void End();
	std::string GetVersionString();
	std::vector<std::string> GetSounds();
	static SoundSource PlaySound3D(SoundBuffer* Sound, Vector3 Position, float MaxDistance, float Pitch = 1.f, float Volume = 1.f, bool Looping = false);

	static SoundSource PlaySound2D(SoundBuffer* Sound, float Pitch = 1.f, float Volume = 1.f, bool Looping = false);
};