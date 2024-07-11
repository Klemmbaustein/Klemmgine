#pragma once
#include <string>
#include <Math/Vector.h>
#include "Subsystem.h"
#include <cstdint>

/**
 * @brief
 * Sound subsystem.
 * 
 * This subsystem controls sound functionality. It does nothing if the configuration is `SERVER`.
 * 
 * OpenAL (the underlying sound system) has a limit of 255 sounds playing at once.
 */
class Sound : public Subsystem
{
public:
	static Sound* SoundSystem;

	/**
	 * @brief
	 * A structure managing a sound created by the PlaySound3D and PlaySound2D functions.
	 */
	struct SoundSource
	{
		SoundSource(unsigned int Buffer, float Pitch, float Volume, bool Looping);
		SoundSource()
		{

		}

		/// Stops the sound.
		void Stop();
		/// Sets the sound's pitch.
		void SetPitch(float NewPitch);
		/// Sets the sound's volume.
		void SetVolume(float NewVolume);
		/// Gets the sound's pitch.
		float GetPitch() const;
		/// Gets the sound's volume.
		float GetVolume() const;
		/// Returns true if the sound is looping.
		bool GetLooping() const;
	private:
		unsigned int Source = 0;  // OpenAL source ID
		float Pitch = 0; // Sound source pitch
		float Volume = 0; // Sound volume (or 'gain')
		bool Looping = false; // If the sound should looping
	};

	/**
	 * @brief
	 * A buffer containing sound data.
	 */
	struct SoundBuffer
	{
		/**
		 * @brief
		 * Loads a sound from a .wav file.
		 * 
		 * OpenAL only supports 8/16 bit sound files with 1 or 2 channels.
		 */
		SoundBuffer(std::string File);

		/**
		 * @brief
		 * Loads a sound from an array of 16 bit integers.
		 * 
		 * @param SoundData
		 * A list of 16 bit integers containing sound data.
		 * 
		 * @param SampleRate
		 * The sample rate of SoundData.
		 * 
		 * @param Mono
		 * True if th sound is a mono sound, false if it's a stereo sound.
		 */
		SoundBuffer(std::vector<int16_t> SoundData, uint32_t SampleRate, bool Mono);

		SoundBuffer(const SoundBuffer&) = delete;

		SoundBuffer(unsigned int Buffer, std::string Name)
		{
			this->Buffer = Buffer;
			this->Name = Name;
		}
		~SoundBuffer();
		unsigned int Buffer;
		std::string Name;
	};

	/// The volume of the sound. ConVar: `sound_volume`.
	float MasterVolume = 1.0f;
	/// If the sound is enabled. ConVar: `sound_enabled`.
	bool Enabled = true;

	Sound();
	virtual ~Sound() override;

	void StopAllSounds();
	void Update() override;
	/// Gets a short string describing the version of the sound system. (OpenAL soft)
	std::string GetVersionString();

	/// Returns a list of strings describing each sound playing right now.
	std::vector<std::string> GetSounds();

	/**
	 * @brief
	 * Plays a sound at a 3d position.
	 * 
	 * @param Sound
	 * The buffer containing the sound info.
	 * 
	 * @param Position
	 * The position where the sound should be played.
	 * 
	 * @param MaxDistance
	 * Controls the falloff of the sound. A higher value means the sound will be heard from further away.
	 * 
	 * @param Pitch
	 * The pitch of the sound.
	 * 
	 * @param Volume
	 * The volume of the sound.
	 * 
	 * @param Looping
	 * True if the sound should keep repeating, false if the sound should only play once.
	 * 
	 * @return
	 * The new sound that was created. 
	 */
	static SoundSource PlaySound3D(SoundBuffer* Sound, Vector3 Position, float MaxDistance, float Pitch = 1.f, float Volume = 1.f, bool Looping = false);

	/**
	 * @brief
	 * Plays a sound without a 3d position..
	 * 
	 * @param Sound
	 * The buffer containing the sound info.
	 * 
	 * The pitch of the sound.
	 * 
	 * @param Volume
	 * The volume of the sound.
	 * 
	 * @param Looping
	 * True if the sound should keep repeating, false if the sound should only play once.
	 * 
	 * @return
	 * The new sound that was created. 
	 */

	static SoundSource PlaySound2D(SoundBuffer* Sound, float Pitch = 1.f, float Volume = 1.f, bool Looping = false);
};