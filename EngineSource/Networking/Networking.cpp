#include "Networking.h"
#include <Objects/WorldObject.h>
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
#include <Engine/Console.h>
#include <Engine/EngineError.h>

namespace Networking
{
	constexpr uint16_t DEFAULT_PORT = 1234;
	constexpr uint32_t TICK_RATE = 64;

	uint16_t reverse_bytes(uint16_t bytes)
	{
		uint16_t aux = 0;
		uint8_t byte;
		int i;

		for (i = 0; i < 16; i += 8)
		{
			byte = (bytes >> i) & 0xff;
			aux |= byte << (16 - 8 - i);
		}
		return aux;
	}

	void InitPacket(IPaddress* Target);
	void InitSockets(uint16_t Port);
	UDPsocket InitSocketFrom(IPaddress* Target);
	SDLNet_SocketSet SocketSet;
	IPaddress srvHost;

	UDPsocket Socket;
	UDPpacket* SentPacket;
	float TickTimer = 0;
	uint64_t Gametick = 0;
	uint64_t NetIDCounter = 0;
}

void Networking::InitPacket(IPaddress* Target)
{
	SentPacket = SDLNet_AllocPacket(Packet::MAX_PACKET_SIZE);
	if (!SentPacket)
	{
		printf("Could not allocate receiving packet\n");
		exit(3);
	}
}

void Networking::InitSockets(uint16_t Port)
{
	if (!(Socket = SDLNet_UDP_Open(Port)))
	{
		printf("Could not create socket\n");
		exit(1);
	}

	IPaddress* myaddress = SDLNet_UDP_GetPeerAddress(Socket, -1);
	if (!myaddress)
	{
		printf("Could not get own port\n");
		exit(2);
	}

	SocketSet = SDLNet_AllocSocketSet(2);
	if (SocketSet == NULL)
	{
		fprintf(stderr, "Couldn't create socket set: %s\n", SDLNet_GetError());
		exit(2);
	}

	auto numused = SDLNet_UDP_AddSocket(SocketSet, Socket);
	if (numused == -1)
	{
		printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
		exit(2);
	}

	SentPacket = SDLNet_AllocPacket(Packet::MAX_PACKET_SIZE);
	if (!SentPacket)
	{
		printf("Could not allocate packet\n");
		exit(2);
	}
	Log::Print(StrUtil::Format("[Net]: Listening on port %i", Port), Log::LogColor::Blue);
}

UDPsocket Networking::InitSocketFrom(IPaddress* Target)
{	
	UDPsocket fret;
	if (!(fret = SDLNet_UDP_Open(0)))
	{
		printf("Could not create socket\n");
		SDLNet_Quit();
		SDL_Quit();
		exit(1);
	}

	//get my address
	IPaddress* myaddress = SDLNet_UDP_GetPeerAddress(fret, -1);
	if (!myaddress)
	{
		printf("Could not get own port\n");
		exit(2);
	}

	UDPsocket rcvS;
	rcvS = SDLNet_UDP_Open(myaddress->port);
	if (!rcvS)
	{
		printf("Could not allocate receiving socket\n");
		exit(4);
	}

	//resolve the address of the server
	srvHost = *Target;
	Log::Print("Connecting to " + IPtoStr(Target));

	SocketSet = SDLNet_AllocSocketSet(2);
	if (SocketSet == NULL)
	{
		fprintf(stderr, "Couldn't create socket set: %s\n", SDLNet_GetError());
		exit(2);
	}
	auto numused = SDLNet_UDP_AddSocket(SocketSet, fret);
	if (numused == -1)
	{
		printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
		exit(2);
	}
	InitPacket(&srvHost);
	return fret;
}

void Networking::SendObjectInfo(WorldObject* obj, void* TargetAddr)
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
		if (i.PType != WorldObject::Property::PropertyType::NetProperty)
		{
			continue;
		}
#if SERVER
		if (i.PropertyOwner == WorldObject::Property::NetOwner::Server || obj->NetOwner != Server::GetClientInfoFromIP(TargetAddr)->ID)
#else
		if (i.PropertyOwner == WorldObject::Property::NetOwner::Client && obj->NetOwner == Client::GetClientID())
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

void Networking::Init()
{
	SDLNet_Init();

#if SERVER
	InitSockets(DEFAULT_PORT);
#else
	Console::RegisterCommand(Console::Command("connect", []() 
		{
			uint16_t Port = DEFAULT_PORT;

			if (Console::CommandArgs().size() > 1)
			{
				Port = std::stoi(Console::CommandArgs()[1]);
			}

			Client::ConnectToServer(Console::CommandArgs()[0], Port);
		}, {Console::Command::Argument("address", NativeType::String), Console::Command::Argument("port", NativeType::Int, true) }));
#endif

#if !SERVER
	Console::RegisterCommand(Console::Command("disconnect", []()
		{
			Client::Disconnect();
		}, {}));
#else
	Console::RegisterCommand(Console::Command("disconnect", []()
		{
			Server::DisconnectPlayer(std::stoi(Console::CommandArgs()[0]));
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
	TickTimer += Performance::DeltaTime;
	if (TickTimer > TickInterval)
	{
		Gametick += (uint64_t)(TickTimer / TickInterval);
		TickTimer = 0;
		HandleTick();
	}
#else
	HandleTick();
	Gametick++;
#endif
}

WorldObject* Networking::SpawnReplicatedObjectFromID(uint32_t ID, Transform Position)
{
	WorldObject* obj = Objects::SpawnObjectFromID(ID, Position, NetIDCounter);
	obj->NetOwner = UINT64_MAX;
#if SERVER
	Server::SpawnObject(ID, NetIDCounter, Position);
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
		(int)Networking::reverse_bytes(((IPaddress*)addr)->port));
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
	return Gametick;
}

std::string Networking::ClientIDToString(uint64_t ID)
{
	if (ID == UINT64_MAX)
	{
		return "Server";
	}
	return "Client " + std::to_string(ID);
}
#endif

// TODO: Have a map of all replicated objects to speed this up.
WorldObject* Networking::GetObjectFromNetID(uint64_t NetID)
{
	for (WorldObject* i : Objects::AllObjects)
	{
		if (i->GetIsReplicated() && i->NetID == NetID)
		{
			return i;
		}
	}
	return nullptr;
}