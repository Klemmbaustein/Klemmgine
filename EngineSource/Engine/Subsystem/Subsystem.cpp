#include "Subsystem.h"
#include <Engine/Log.h>
#include <array>

std::vector<Subsystem*> Subsystem::LoadedSystems;

Subsystem::Subsystem()
{
}

Subsystem::~Subsystem()
{
	Print("Unloaded subsystem.", ErrorLevel::Note);
}

void Subsystem::Update()
{
}

std::array<Vector3, 4> LogLevelColors =
{
	Log::LogColor::White,
	Log::LogColor::Yellow,
	Log::LogColor::Red,
	Log::LogColor::Gray,
};

void Subsystem::Print(std::string Msg, ErrorLevel MsgErrLvl)
{
	if (Prefix.empty())
	{
		std::string ResizedName = Name;
		ResizedName.resize(8, ' ');

		if (std::strlen(SystemType))
		{
			Prefix = "[" + std::string(SystemType) + "]: [" + ResizedName + "]: ";
		}
		else
		{
			Prefix = "[" + ResizedName + "]: ";
		}
	}
	Log::PrintMultiLine(Msg, LogLevelColors[(size_t)MsgErrLvl], Prefix);
}

void Subsystem::Load(Subsystem* System)
{
	LoadedSystems.push_back(System);
	System->Print("Loaded subsystem.", ErrorLevel::Note);
}

void Subsystem::Unload(Subsystem* System)
{
	for (size_t i = 0; i < LoadedSystems.size(); i++)
	{
		if (LoadedSystems[i] == System)
		{
			LoadedSystems.erase(LoadedSystems.begin() + i);
			return;
		}
	}
}

Subsystem* Subsystem::GetSubsystemByName(std::string Name)
{
	for (Subsystem* System : LoadedSystems)
	{
		if (System->Name == Name)
		{
			return System;
		}
	}
	return nullptr;
}

void Subsystem::UpdateSubsystems()
{
	for (Subsystem* System : LoadedSystems)
	{
		System->Update();
	}
}

void Subsystem::DestroyAll()
{
	for (Subsystem* System : LoadedSystems)
	{
		delete System;
	}
	LoadedSystems.clear();
}
