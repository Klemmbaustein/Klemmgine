#include "Networking.h"
#include <Objects/SceneObject.h>
#if !EDITOR
#include <iostream>
#include <Engine/Log.h>
#include <SDL_net.h>
#include <Engine/Input.h>
#include <Engine/Stats.h>
#include <Engine/Utility/StringUtility.h>
#include "Packet.h"
#include <Networking/NetworkingInternal.h>
#include <Networking/Client.h>
#include "NetworkEvent.h"
#include "Server.h"
#include <Engine/Subsystem/Console.h>
#include <Engine/EngineError.h>
#if _WIN32
// Undefine macros first defined in SDL_net.h to avoid warnings.
#undef INADDR_ANY
#undef INADDR_BROADCAST
#undef INADDR_NONE

#include <winsock.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

namespace Networking
{
	constexpr uint32_t TICK_RATE = 64;

	void InitPacket(IPaddress* Target);
	void InitSockets(uint16_t Port);
	UDPsocket InitSocketFrom(IPaddress* Target);
	SDLNet_SocketSet SocketSet;
	IPaddress srvHost;

	UDPsocket Socket;
	UDPpacket* PacketData;
	float TickTimer = 0;
	uint64_t GameTick = 0;
	bool IsServerTickFrame = false;
	uint64_t NetIDCounter = 0;

	static uint16_t DefaultPort = 0;
}

void Networking::InitPacket(IPaddress* Target)
{
	PacketData = SDLNet_AllocPacket(Packet::MAX_PACKET_SIZE);
	if (!PacketData)
	{
		Log::Print("Could not allocate receiving packet", Log::LogColor::Red);
	}
}

void Networking::InitSockets(uint16_t Port)
{
	if (!(Socket = SDLNet_UDP_Open(Port)))
	{
		Log::Print(StrUtil::Format("Could not create socket on port '%i'", int(Port)), Log::LogColor::Red);
		return;
	}

	IPaddress* CurrentAddress = SDLNet_UDP_GetPeerAddress(Socket, -1);
	if (!CurrentAddress)
	{
		Log::Print("Could not get own port", Log::LogColor::Red);
		return;
	}

	SocketSet = SDLNet_AllocSocketSet(2);
	if (SocketSet == NULL)
	{
		Log::Print(StrUtil::Format("Couldn't create socket set: %s\n", SDLNet_GetError()), Log::LogColor::Red);
		return;
	}

	auto UsedSockets = SDLNet_UDP_AddSocket(SocketSet, Socket);
	if (UsedSockets == -1)
	{
		Log::Print(StrUtil::Format("SDLNet_AddSocket: %s\n", SDLNet_GetError()), Log::LogColor::Red);
		return;
	}

	PacketData = SDLNet_AllocPacket(Packet::MAX_PACKET_SIZE);
	if (!PacketData)
	{
		Log::Print("Could not allocate packet", Log::LogColor::Red);
		return;
	}
	Log::Print(StrUtil::Format("[Net]: Starting server on port %i", Port), Log::LogColor::Blue);
}

UDPsocket Networking::InitSocketFrom(IPaddress* Target)
{	
	UDPsocket fret;
	if (!(fret = SDLNet_UDP_Open(0)))
	{
		Log::Print("Could not create socket", Log::LogColor::Red);
		SDLNet_Quit();
		SDL_Quit();
		return nullptr;
	}

	IPaddress* CurrentAddress = SDLNet_UDP_GetPeerAddress(fret, -1);
	if (!CurrentAddress)
	{
		Log::Print("Could not get own port", Log::LogColor::Red);
		return nullptr;
	}

	UDPsocket rcvS;
	rcvS = SDLNet_UDP_Open(CurrentAddress->port);
	if (!rcvS)
	{
		Log::Print(StrUtil::Format("Could not allocate receiving socket: %s\n", SDLNet_GetError()), Log::LogColor::Red);
		return nullptr;
	}

	//resolve the address of the server
	srvHost = *Target;
	Log::Print("Connecting to " + IPtoStr(Target));

	SocketSet = SDLNet_AllocSocketSet(2);
	if (SocketSet == NULL)
	{
		Log::Print(StrUtil::Format("Couldn't create socket set: %s\n", SDLNet_GetError()), Log::LogColor::Red);
		return nullptr;
	}
	auto UsedSockets = SDLNet_UDP_AddSocket(SocketSet, fret);
	if (UsedSockets == -1)
	{
		Log::Print(StrUtil::Format("SDLNet_AddSocket: %s\n", SDLNet_GetError()), Log::LogColor::Red);
		return nullptr;
	}
	InitPacket(&srvHost);
	return fret;
}

void Networking::SendObjectInfo(SceneObject* obj, void* TargetAddr)
{
	if (!obj->GetIsReplicated())
	{
		return;
	}

	if (Client::GetClientID() == obj->NetOwner
		|| (Client::GetClientID() == Networking::ServerID
			&& Server::GetClientInfoFromIP(TargetAddr)->ID != obj->NetOwner))
	{
		Packet p;
		p.Data =
		{
			(uint8_t)Packet::PacketType::ValueUpdate,
		};

		p.Write(obj->NetID);
		p.AppendStringToData("_pos=" + obj->GetTransform().Position.ToString());
		p.AppendStringToData(";_rot=" + obj->GetTransform().Rotation.ToString());
		p.AppendStringToData(";_scl=" + obj->GetTransform().Scale.ToString());
		p.AppendStringToData(";_owner=" + std::to_string(obj->NetOwner));
		p.Send(TargetAddr);
	}
	else if (Client::GetClientID() != Networking::ServerID)
	{
		return;
	}

	Packet Properties;
	for (auto& i : obj->Properties)
	{
		if (i.PType != SceneObject::Property::PropertyType::NetProperty)
		{
			continue;
		}
#if SERVER
		if (i.PropertyOwner == SceneObject::Property::NetOwner::Server || obj->NetOwner != Server::GetClientInfoFromIP(TargetAddr)->ID)
#else
		if (i.PropertyOwner == SceneObject::Property::NetOwner::Client && obj->NetOwner == Client::GetClientID())
#endif
		{
			if (Properties.Data.empty())
			{
				Properties.Data =
				{
					(uint8_t)Packet::PacketType::ValueUpdate,
				};

				Properties.Data.resize(9);
				memcpy(&Properties.Data[1], &obj->NetID, sizeof(size_t));
			}

			Properties.AppendStringToData(i.Name + "=" + i.ValueToString(obj) + ";");
		}
	}
	if (!Properties.Data.empty())
	{
		Properties.Send(TargetAddr);
	}
}

void Networking::Init(uint16_t DefaultPort)
{
	Networking::DefaultPort = DefaultPort;
	SDLNet_Init();

#if SERVER
	InitSockets(DefaultPort);
#else
	Console::ConsoleSystem->RegisterCommand(Console::Command("connect", []()
		{
			uint16_t Port = Networking::DefaultPort;

			if (Console::ConsoleSystem->CommandArgs().size() > 1)
			{
				Port = std::stoi(Console::ConsoleSystem->CommandArgs()[1]);
			}

			Client::ConnectToServer(Console::ConsoleSystem->CommandArgs()[0], Port);
		}, {Console::Command::Argument("address", NativeType::String), Console::Command::Argument("port", NativeType::Int, true) }));
#endif

#if !SERVER
	Console::ConsoleSystem->RegisterCommand(Console::Command("disconnect", []()
		{
			Client::Disconnect();
		}, {}));
#else
	Console::ConsoleSystem->RegisterCommand(Console::Command("disconnect", []()
		{
			Server::DisconnectPlayer(std::stoi(Console::ConsoleSystem->CommandArgs()[0]));
		}, { Console::Command::Argument("player_uid", NativeType::Int) }));
#endif
#if SERVER
	Server::Init();
	Packet::Init();
#endif

}

void Networking::HandleTick()
{
	ReceivePackets();
#if !SERVER
	Client::Update();
#else
	Server::Update();
#endif
	NetworkEvent::Update();
}

void Networking::ReceivePackets()
{
	auto p = Packet::Receive();

	for (auto& i : p)
	{
		i.second.EvaluatePacket();
	}
}

void Networking::Update()
{
#if !SERVER
	float TickInterval = 1.0f / (float)TICK_RATE;
	TickTimer += Stats::DeltaTime;
	if (TickTimer > TickInterval)
	{
		GameTick += (uint64_t)(TickTimer / TickInterval);
		TickTimer = 0;
		IsServerTickFrame = true;
		HandleTick();
	}
	else
	{
		IsServerTickFrame = false;
	}
#else
	HandleTick();
	IsServerTickFrame = true;
	GameTick++;
#endif
}

SceneObject* Networking::SpawnReplicatedObjectFromID(uint32_t ID, Transform Position)
{
	SceneObject* obj = Objects::SpawnObjectFromID(ID, Position, NetIDCounter);
	obj->NetOwner = UINT64_MAX;
#if SERVER
	Server::SpawnObject(ID, NetIDCounter, Position, obj->GetPropertiesAsString());
#endif
	NetIDCounter++;
	return obj;
}

std::string Networking::IPtoStr(void* addr)
{
	Uint8* parts = (Uint8*)&((IPaddress*)addr)->host;
	return StrUtil::Format("%i.%i.%i.%i:%i",
		(int)parts[0],
		(int)parts[1],
		(int)parts[2],
		(int)parts[3],
		(int)ntohs(((IPaddress*)addr)->port));
}

void Networking::Exit()
{
	Client::Exit();
}

bool Networking::IPEqual(void* ip1, void* ip2)
{
	IPaddress ip1addr = *(IPaddress*)ip1;
	IPaddress ip2addr = *(IPaddress*)ip2;
	return ip1addr.host == ip2addr.host && ip1addr.port == ip2addr.port;
}

uint32_t Networking::GetTickRate()
{
	return TICK_RATE;
}

uint64_t Networking::GetGameTick()
{
	return GameTick;
}

bool Networking::GetIsServerTickFrame()
{
	return IsServerTickFrame;
}

float Networking::GetTickDelta()
{
	return 1.0f / (float)TICK_RATE;
}

std::string Networking::ClientIDToString(uint64_t ID)
{
	if (ID == UINT64_MAX)
	{
		return "Server";
	}
	return "Client " + std::to_string(ID);
}
uint16_t Networking::GetDefaultPort()
{
	return DefaultPort;
}
#endif

// TODO: Have a map of all replicated objects to speed this up.
SceneObject* Networking::GetObjectFromNetID(uint64_t NetID)
{
	for (SceneObject* i : Objects::AllObjects)
	{
		if (i->GetIsReplicated() && i->NetID == NetID)
		{
			return i;
		}
	}
	return nullptr;
}