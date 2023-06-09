#include "Sound.h"
#include <AL/al.h>
#include <Engine/FileUtility.h>
#include <AL/alc.h>
#include "AL/alext.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <Engine/Log.h>
#include <Rendering/Camera/Camera.h>
#include <World/Assets.h>
#include <iostream>
#include <World/Graphics.h>
#include <Engine/Console.h>
#include <filesystem>
#include <Engine/EngineError.h>

struct Source
{
	Source(ALuint AudioSource, float Pitch, float Volume, bool Looping, bool Is3D, Vector3 Location, float Distance, std::string Name)
	{
		this->AudioSource = AudioSource;
		this->Pitch = Pitch;
		this->Volume = Volume;
		this->Looping = Looping;
		this->Is3D = Is3D;
		this->Location = Location;
		this->Distance = Distance;
		this->name = Name;
	}
	float Pitch = 1.f;
	float Volume = 1.f;
	bool Looping = false;
	bool Is3D = false;
	Vector3 Location;
	float Distance;
	ALuint AudioSource;
	std::string name;
};
std::vector<Source> CurrentSources;
class SoundException : public std::exception
{
	std::string Message;
public:
	SoundException(std::string Message)
	{
		this->Message = "SoundException: " + Message;
	}

	const char* what() const throw ()
	{
		return Message.c_str();
	}
};
std::vector<ALuint> Buffers;
ALCdevice* CurrentDevice;
ALCcontext* ALContext;
namespace Sound
{
	float MasterVolume = 1.f;
}
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
int InitAL(char*** argv, int* argc)
{
	const ALCchar* name;
	CurrentDevice;

	/* Open and initialize a device */
	CurrentDevice = NULL;
	if (argc && argv && *argc > 1 && strcmp((*argv)[0], "-device") == 0)
	{
		CurrentDevice = alcOpenDevice((*argv)[1]);
		if (!CurrentDevice)
			fprintf(stderr, "Failed to open \"%s\", trying default\n", (*argv)[1]);
		(*argv) += 2;
		(*argc) -= 2;
	}
	if (!CurrentDevice)
		CurrentDevice = alcOpenDevice(NULL);
	if (!CurrentDevice)
	{
		fprintf(stderr, "Could not open a device!\n");
		return 1;
	}

	ALContext = alcCreateContext(CurrentDevice, NULL);
	if (ALContext == NULL || alcMakeContextCurrent(ALContext) == ALC_FALSE)
	{
		if (ALContext != NULL)
			alcDestroyContext(ALContext);
		alcCloseDevice(CurrentDevice);
		fprintf(stderr, "Could not set a context!\n");
		return 1;
	}

	name = NULL;
	if (alcIsExtensionPresent(CurrentDevice, "ALC_ENUMERATE_ALL_EXT"))
		name = alcGetString(CurrentDevice, ALC_ALL_DEVICES_SPECIFIER);
	if (!name || alcGetError(CurrentDevice) != AL_NO_ERROR)
		name = alcGetString(CurrentDevice, ALC_DEVICE_SPECIFIER);
	printf("Opened \"%s\"\n", name);

	return 0;
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

namespace Sound
{
	void Update3DVolumeOfSound(const Source& CurrentSource, const Vector3& Position)
	{
		float Distance = (CurrentSource.Location - Position).Length();
		Distance = Distance * Distance * (CurrentSource.Distance / 100.0f) / 100.f;
		alSourcef(CurrentSource.AudioSource, AL_GAIN, std::max(CurrentSource.Volume - Distance, 0.0f));
	}

	void StopAllSounds()
	{
		for (Source s : CurrentSources)
		{
			if (alIsSource(s.AudioSource))
			{
				alDeleteSources(1, &s.AudioSource);
			}
		}
		CurrentSources.clear();
	}
	void Update()
	{
		Vector3 ForwardVec, PositionVec;
		if (Graphics::MainCamera)
		{
			ForwardVec = Vector3::GetForwardVector(Graphics::MainCamera->Rotation);
			PositionVec = Graphics::MainCamera->Position;
		}
		else
		{
			ForwardVec = Vector3(0, 0, 1);
		}
		Vector3 Forward[2] = { ForwardVec, Vector3(0, 1, 0)};
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
				else
				{
				}
			}
		}
	}
	void Init()
	{
		ALCdevice* device = alcOpenDevice(NULL);
		ENGINE_ASSERT(device != NULL, "Cannot open sound card");

		ALCcontext* context = alcCreateContext(device, NULL);
		ENGINE_ASSERT(context != NULL, "Cannot open OpenAL Context");

		alcMakeContextCurrent(context);
		alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
		alListenerf(AL_GAIN, 1.1f);
		Console::RegisterCommand(Console::Command("playsound", []()
			{
				auto LoadedBuffer = LoadSound(Console::CommandArgs()[0]);
				if (LoadedBuffer)
				{
					PlaySound2D(LoadedBuffer);
					Console::ConsoleLog("Played sound " + Console::CommandArgs()[0]);
					return;
				}
				Console::ConsoleLog("Sound " + Console::CommandArgs()[0] + " doesn't exist!", Console::E_ERROR);
			}, {Console::Command::Argument("sound", Type::E_STRING)}));
		Console::RegisterConVar(Console::Variable("soundvolume", Type::E_FLOAT, &MasterVolume, nullptr));

	}
	void End()
	{
		alcCloseDevice(CurrentDevice);
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(ALContext);
		Log::Print("Sound: Shutting down", Vector3(1, 1, 0));
	}
	void SetSoundVolume(float NewVolume)
	{
		MasterVolume = NewVolume;
	}
	float GetSoundVolume()
	{
		return MasterVolume;
	}
	std::string GetVersionString()
	{
		return alGetString(AL_VERSION);
	}
	std::vector<std::string> GetSounds()
	{
		std::vector<std::string> Sounds;
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
		return Sounds;
	}
	SoundSource PlaySound3D(SoundBuffer* Sound, Vector3 Position, float MaxDistance, float Pitch, float Volume, bool Looping)
	{
		ALuint NewSource;
		Vector3 PositionVec;
		alGenSources(1, &NewSource);
		if (Graphics::MainCamera)
		{
			PositionVec = Graphics::MainCamera->Position;
		}
		alSourcef(NewSource, AL_PITCH, Pitch);
		alSourcef(NewSource, AL_GAIN, Volume);
		alSource3f(NewSource, AL_POSITION, PositionVec.X, PositionVec.Y, PositionVec.Z);
		alSource3f(NewSource, AL_POSITION, Position.X, Position.Y, Position.Z);
		alSource3f(NewSource, AL_VELOCITY, 0, 0, 0);
		alSourcei(NewSource, AL_LOOPING, Looping);
		alSourcei(NewSource, AL_BUFFER, Sound->Buffer);
		alSourcePlay(NewSource);
		CurrentSources.push_back(Source(NewSource, Pitch, Volume, Looping, true, Position, MaxDistance, Sound->Name + " (ID: " + std::to_string(Sound->Buffer) + ")"));
		Update3DVolumeOfSound(CurrentSources[CurrentSources.size() - 1], PositionVec);
		return SoundSource(NewSource, Pitch, Volume, Looping);
	}

	SoundSource PlaySound2D(SoundBuffer* Sound, float Pitch, float Volume, bool Looping)
	{
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
	}

	SoundBuffer* LoadSound(std::string File)
	{
		File.append(".wav");
		int channel, sampleRate, bps, size;
		std::string FileAsset = Assets::GetAsset(File);
		if (!std::filesystem::exists(FileAsset))
		{
			Log::Print("Could not load sound file: " + File);
			return nullptr;
		}
		char* data = loadWAV(FileAsset.c_str(), channel, sampleRate, bps, size);
		
		unsigned int bufferid = 0, format = 0;
		alGenBuffers(1, &bufferid);
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
			else if(bps == 16)
			{
				FoundValidFormat = true;
				format = AL_FORMAT_STEREO16;
			}
		}
		if (!FoundValidFormat)
		{
			Log::Print("Error: Tried to load " + File + " but it's sound format \"" + std::to_string(channel) + " channel(s), "
				+ std::to_string(bps) + " bit\" is not supported!", Vector3(1, 0, 0));
			Log::Print("Error: Supported formats are: 1/2 channel(s), 8/16bit", Vector3(1, 0, 0));
			{
				return nullptr;
			}
		}
		alBufferData(bufferid, format, data, size, sampleRate);
		delete data;
		return new SoundBuffer(bufferid, File);
	}
	SoundBuffer::~SoundBuffer()
	{
		alDeleteBuffers(1, &Buffer);
	}
	SoundSource::SoundSource(unsigned int Buffer, float Pitch, float Volume, bool Looping)
	{
		this->Source = Buffer;
		this->Pitch = Pitch;
		this->Volume = Volume;
		this->Looping = Looping;
	}
	void SoundSource::Stop()
	{
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
	}
	void SoundSource::SetPitch(float NewPitch)
	{
		if(alIsSource(Source))
		alSourcef(Source, AL_PITCH, NewPitch);
	}
	void SoundSource::SetVolume(float NewVolume)
	{
		if (alIsSource(Source))
		alSourcef(Source, AL_GAIN, NewVolume);
	}
	float SoundSource::GetPitch()
	{
		return Pitch;
	}
	float SoundSource::GetVolume()
	{
		return Volume;
	}
	bool SoundSource::GetLooping()
	{
		return Looping;
	}
}