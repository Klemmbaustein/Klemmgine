#pragma once
#include <string>
#include <Math/Vector.h>
namespace Sound
{
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
		SoundBuffer(unsigned int Buffer, std::string Name)
		{
			this->Buffer = Buffer;
			this->Name = Name;
		}
		~SoundBuffer();
		unsigned int Buffer;
		std::string Name;
	};
	void StopAllSounds();
	void Update();
	void Init();
	void End();
	void SetSoundVolume(float NewVolume);
	float GetSoundVolume();
	std::string GetVersionString();
	std::vector<std::string> GetSounds();
	SoundSource PlaySound3D(SoundBuffer* Sound, Vector3 Position, float MaxDistance, float Pitch = 1.f, float Volume = 1.f, bool Looping = false);

	SoundSource PlaySound2D(SoundBuffer* Sound, float Pitch = 1.f, float Volume = 1.f, bool Looping = false);

	SoundBuffer* LoadSound(std::string File);
}