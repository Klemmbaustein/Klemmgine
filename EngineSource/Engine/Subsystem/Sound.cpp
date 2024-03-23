#include "Sound.h"
#if !SERVER
#include <AL/al.h>
#include <Engine/Utility/FileUtility.h>
#include <AL/alc.h>
#include "AL/alext.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <Engine/Log.h>
#include <Rendering/Camera/Camera.h>
#include <Engine/File/Assets.h>
#include <iostream>
#include <Rendering/Graphics.h>
#include <filesystem>
#include <Engine/EngineError.h>
#include <Engine/Subsystem/Console.h>

struct Source
{
	Source(ALuint AudioSource, float Pitch, float Volume, bool Looping, bool Is3D, Vector3 Position, float Distance, std::string Name)
	{
		this->AudioSource = AudioSource;
		this->Pitch = Pitch;
		this->Volume = Volume;
		this->Looping = Looping;
		this->Is3D = Is3D;
		this->Position = Position;
		this->Distance = Distance;
		this->name = Name;
	}
	float Pitch = 1.f;
	float Volume = 1.f;
	bool Looping = false;
	bool Is3D = false;
	Vector3 Position;
	float Distance;
	ALuint AudioSource;
	std::string name;
};

std::vector<Source> CurrentSources;
std::vector<ALuint> Buffers;
ALCdevice* CurrentDevice;
ALCcontext* ALContext;
Sound* Sound::SoundSystem = nullptr;

std::vector<std::string> GetAvaliableDevices(ALCdevice* device)
{
	const ALCchar* devices = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
	const char* ptr = devices;
	std::vector<std::string> devicesVec;

	do
	{
		devicesVec.push_back(std::string(ptr));
		ptr += devicesVec.back().size() + 1;
	} while (*(ptr + 1) != '\0');

	return devicesVec;
}

bool IsBigEndian()
{
	int a = 1;
	return !((char*)&a)[0];
}

static void printDeviceList(const char* list)
{
	if (!list || *list == '\0')
		printf("    !!! none !!!\n");
	else do {
		printf("    %s\n", list);
		list += strlen(list) + 1;
	} while (*list != '\0');
}
int convertToInt(char* buffer, int len)
{
	int a = 0;
	if (!IsBigEndian())
		for (int i = 0; i < len; i++)
			((char*)&a)[i] = buffer[i];
	else
		for (int i = 0; i < len; i++)
			((char*)&a)[3 - i] = buffer[i];
	return a;
}

char* loadWAV(const char* fn, int& chan, int& samplerate, int& bps, int& size)
{
	char buffer[4];
	std::ifstream in(fn, std::ios::binary);
	in.read(buffer, 4);
	if (strncmp(buffer, "RIFF", 4) != 0)
	{
		std::cout << "this is not a valid WAVE file (" << fn << ")" << std::endl;
		return NULL;
	}
	in.read(buffer, 4);
	in.read(buffer, 4);      //WAVE
	in.read(buffer, 4);      //fmt
	in.read(buffer, 4);      //16
	in.read(buffer, 2);      //1
	in.read(buffer, 2);
	chan = convertToInt(buffer, 2);
	in.read(buffer, 4);
	samplerate = convertToInt(buffer, 4);
	in.read(buffer, 4);
	in.read(buffer, 2);
	in.read(buffer, 2);
	bps = convertToInt(buffer, 2);
	in.read(buffer, 4);      //data
	in.read(buffer, 4);
	size = convertToInt(buffer, 4);
	char* data = new char[size];
	in.read(data, size);
	return data;
}
#endif
#if !SERVER
void Update3DVolumeOfSound(const Source& CurrentSource, const Vector3& Position)
{
	float Distance = (CurrentSource.Position - Position).Length();
	float Volume = (CurrentSource.Distance - Distance) / CurrentSource.Distance;
	alSourcef(CurrentSource.AudioSource, AL_GAIN, std::max(CurrentSource.Volume * Volume, 0.0f));
}
#endif
void Sound::StopAllSounds()
{
#if !SERVER
	for (Source s : CurrentSources)
	{
		if (alIsSource(s.AudioSource))
		{
			alDeleteSources(1, &s.AudioSource);
		}
	}
	CurrentSources.clear();
#endif
}

void Sound::Update()
{
#if !SERVER
	Vector3 ForwardVec, PositionVec = Vector3(0, 100000, 0);
	if (Graphics::MainCamera)
	{
		ForwardVec = Vector3::GetForwardVector(Graphics::MainCamera->Rotation);
		PositionVec = Graphics::MainCamera->Position;
	}
	else
	{
		ForwardVec = Vector3(0, 0, 1);
	}
	Vector3 Forward[2] = { ForwardVec, Vector3(0, 1, 0) };
	alListenerf(AL_GAIN, MasterVolume);
	alListenerfv(AL_POSITION, &PositionVec.X);
	alListenerfv(AL_ORIENTATION, &Forward[0].X);
	for (int i = 0; i < CurrentSources.size(); i++)
	{
		ALenum SoundStatus;
		alGetSourcei(CurrentSources[i].AudioSource, AL_SOURCE_STATE, &SoundStatus);
		if (SoundStatus != AL_PLAYING)
		{
			alDeleteSources(1, &CurrentSources[i].AudioSource);
			CurrentSources.erase(CurrentSources.begin() + i);
			break;
		}
		else
		{
			if (CurrentSources[i].Is3D)
			{
				Update3DVolumeOfSound(CurrentSources[i], PositionVec);
			}
		}
	}
#endif
}
Sound::Sound()
{
	SoundSystem = this;
	Name = "SoundSys";
#if !SERVER
	ALCdevice* device = alcOpenDevice(NULL);
	ENGINE_ASSERT(device != NULL, "Cannot open sound card");

	ALCcontext* context = alcCreateContext(device, NULL);
	ENGINE_ASSERT(context != NULL, "Cannot open OpenAL Context");

	alcMakeContextCurrent(context);
	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	alListenerf(AL_GAIN, 1.1f);
	Console::ConsoleSystem->RegisterCommand(Console::Command("play_sound", []()
		{
			auto LoadedBuffer = new SoundBuffer(Console::ConsoleSystem->CommandArgs()[0]);
			if (LoadedBuffer)
			{
				PlaySound2D(LoadedBuffer);
				Console::ConsoleSystem->Print("Played sound " + Console::ConsoleSystem->CommandArgs()[0]);
				return;
			}
			Console::ConsoleSystem->Print("Sound " + Console::ConsoleSystem->CommandArgs()[0] + " doesn't exist!", ErrorLevel::Error);
		}, { Console::Command::Argument("sound", NativeType::String) }));
	Console::ConsoleSystem->RegisterConVar(Console::Variable("sound_volume", NativeType::Float, &MasterVolume, nullptr));

	Update();
#endif
}
Sound::~Sound()
{
#if !SERVER
	alcCloseDevice(CurrentDevice);
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(ALContext);
#endif
}
std::string Sound::GetVersionString()
{
#if !SERVER
	return alGetString(AL_VERSION);
#else
	return "No sound";
#endif
}
std::vector<std::string> Sound::GetSounds()
{
	std::vector<std::string> Sounds;
#if !SERVER
	for (Source& s : CurrentSources)
	{
		if (s.Is3D)
		{
			Sounds.push_back(s.name + " (Is 3D, distance = " + std::to_string(s.Distance) + ")");
		}
		else
		{
			Sounds.push_back(s.name);
		}
	}
#endif
	return Sounds;
}
Sound::SoundSource Sound::PlaySound3D(SoundBuffer* Sound, Vector3 Position, float MaxDistance, float Pitch, float Volume, bool Looping)
{
#if !SERVER
	ALuint NewSource;
	Vector3 PositionVec = Vector3(0, 10000, 0);
	alGenSources(1, &NewSource);
	if (Graphics::MainCamera)
	{
		PositionVec = Graphics::MainCamera->Position;
	}
	alSourcef(NewSource, AL_PITCH, Pitch);
	alSourcef(NewSource, AL_GAIN, Volume);
	alSource3f(NewSource, AL_POSITION, Position.X, Position.Y, Position.Z);
	alSource3f(NewSource, AL_VELOCITY, 0, 0, 0);
	alSourcef(NewSource, AL_ROLLOFF_FACTOR, 0.0f);
	alSourcei(NewSource, AL_LOOPING, Looping);
	alSourcei(NewSource, AL_BUFFER, Sound->Buffer);
	alSourcePlay(NewSource);
	CurrentSources.push_back(Source(NewSource, Pitch, Volume, Looping, true, Position, MaxDistance, Sound->Name + " (ID: " + std::to_string(Sound->Buffer) + ")"));
	Update3DVolumeOfSound(CurrentSources[CurrentSources.size() - 1], PositionVec);
	return SoundSource(NewSource, Pitch, Volume, Looping);
#endif
	return SoundSource(0, 0, 0, false);
}

Sound::SoundSource Sound::PlaySound2D(SoundBuffer* Sound, float Pitch, float Volume, bool Looping)
{
#if !SERVER
	ALuint NewSource;
	alGenSources(1, &NewSource);
	alSourcef(NewSource, AL_PITCH, Pitch);
	alSourcef(NewSource, AL_GAIN, Volume);
	alSourcei(NewSource, AL_SOURCE_RELATIVE, AL_TRUE);
	alSource3f(NewSource, AL_POSITION, 0, 0, 0);
	alSourcei(NewSource, AL_LOOPING, Looping);
	alSourcei(NewSource, AL_BUFFER, Sound->Buffer);
	alSourcePlay(NewSource);
	CurrentSources.push_back(Source(NewSource, Pitch, Volume, Looping, false, Vector3(), 0, Sound->Name + " (ID: " + std::to_string(Sound->Buffer) + ")"));
	return SoundSource(NewSource, Pitch, Volume, Looping);
#endif
	return SoundSource(0, 0, 0, false);
}

Sound::SoundBuffer::SoundBuffer(std::string File)
{
#if !SERVER
	Buffer = 0;
	File.append(".wav");
	int channel, sampleRate, bps, size;
	std::string FileAsset = Assets::GetAsset(File);
	if (!std::filesystem::exists(FileAsset))
	{
		SoundSystem->Print("Could not load sound file: " + File);
		return;
	}
	char* data = loadWAV(FileAsset.c_str(), channel, sampleRate, bps, size);

	unsigned int format = 0;
	alGenBuffers(1, &Buffer);
	bool FoundValidFormat = false;
	if (channel == 1)
	{
		if (bps == 8)
		{
			format = AL_FORMAT_MONO8;
			FoundValidFormat = true;
		}
		else if (bps == 16)
		{
			format = AL_FORMAT_MONO16;
			FoundValidFormat = true;
		}
	}
	else
	{
		if (bps == 8)
		{
			format = AL_FORMAT_STEREO8;
			FoundValidFormat = true;
		}
		else if (bps == 16)
		{
			FoundValidFormat = true;
			format = AL_FORMAT_STEREO16;
		}
	}
	if (!FoundValidFormat)
	{
		SoundSystem->Print("Tried to load " + File + " but it's sound format \"" + std::to_string(channel) + " channel(s), "
			+ std::to_string(bps) + " bit\" is not supported!", ErrorLevel::Error);
		SoundSystem->Print("Supported formats are: 1/2 channel(s), 8/16bit", ErrorLevel::Error);
		{
			return;
		}
	}
	alBufferData(Buffer, format, data, size, sampleRate);
	Name = File;
	delete data;
#endif
}

Sound::SoundBuffer::~SoundBuffer()
{
#if !SERVER
	alDeleteBuffers(1, &Buffer);
#endif
}

Sound::SoundSource::SoundSource(unsigned int Buffer, float Pitch, float Volume, bool Looping)
{
	this->Source = Buffer;
	this->Pitch = Pitch;
	this->Volume = Volume;
	this->Looping = Looping;
}

void Sound::SoundSource::Stop()
{
#if !SERVER
	alSourceStop(Source);
	alDeleteSources(1, &Source);
	for (size_t i = 0; i < CurrentSources.size(); i++)
	{
		if (CurrentSources[i].AudioSource == Source)
		{
			alDeleteSources(1, &CurrentSources[i].AudioSource);
			CurrentSources.erase(CurrentSources.begin() + i);
			break;
		}
	}
#endif
}

void Sound::SoundSource::SetPitch(float NewPitch)
{
#if !SERVER
	if (alIsSource(Source))
		alSourcef(Source, AL_PITCH, NewPitch);
#endif
}

void Sound::SoundSource::SetVolume(float NewVolume)
{
#if !SERVER
	if (alIsSource(Source))
		alSourcef(Source, AL_GAIN, NewVolume);
#endif
}

float Sound::SoundSource::GetPitch()
{
	return Pitch;
}

float Sound::SoundSource::GetVolume()
{
	return Volume;
}

bool Sound::SoundSource::GetLooping()
{
	return Looping;
}
