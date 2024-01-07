#if !EDITOR
#include "Packet.h"
#include <Engine/Log.h>
#include "Networking.h"
#include "NetworkEvent.h"
#include "NetworkingInternal.h"
#include <iostream>
#include <Networking/Server.h>
#include <Networking/Client.h>
#include <Engine/Utility/StringUtility.h>
#include <Objects/WorldObject.h>
#include <Engine/Scene.h>
#include <thread>
#include <mutex>
#include <condition_variable>

const int Packet::MAX_PACKET_SIZE = 512;
uint64_t Packet::PacketID = 0;
namespace pkt
{
	std::thread* pktThread = nullptr;
	static bool RecvPacket = false;
	std::condition_variable cv;
	std::mutex mutex;
	std::map<uint64_t, Packet> lines;
	UDPpacket* threadPacket;
}
using namespace pkt;

void Packet::EvaluatePacket()
{
	if (Data.empty())
	{
		return;
	}

	StreamPos = 1;

#if SERVER
	Server::HandlePacket(this);
#endif
	switch ((PacketType)Data[0])
	{
	case PacketType::ConnectRequest:
#if SERVER
		Server::OnConnectRequestReceived(*this);
#endif
		break;
	case PacketType::ConnectionAccept:
#if !SERVER
		Client::OnConnected(this);
#endif
		break;
	case PacketType::ValueUpdate:
	{
		if (Data.size() < sizeof(uint64_t) + 1)
		{
			break;
		}

		uint64_t ObjID = 0;
		Read(ObjID);
		std::string Value;
		for (size_t i = 1 + sizeof(ObjID); i < Data.size(); i++)
		{
			if (Data[i] == 0)
			{
				break;
			}
			Value.append({ (char)Data[i] });
		}

		auto Values = StrUtil::SeperateString(Value, ';');

		for (auto& i : Values)
		{
			Client::HandleValueUpdate(ObjID,
				i.substr(0, i.find_first_of("=")),
				i.substr(i.find_first_of("=") + 1), true);
		}
		break;
	}
	case PacketType::DisconnectRequest:
#if SERVER
		Server::DisconnectPlayer(FromAddr);
#else
		Client::Disconnect();
#endif
		break;
	case PacketType::SpawnObject:
#if !SERVER
	{
		if (!Client::GetIsConnected())
		{
			return;
		}
		if (Data.size() < (1 + sizeof(int32_t) + sizeof(uint64_t) + sizeof(uint64_t) + sizeof(Transform)))
		{
			break;
		}

		int32_t ObjTypeID = 0;
		uint64_t ObjNetID = 0;
		uint64_t ObjOwnerID = 0;
		Transform SpawnTransform = Transform();
		Read(ObjTypeID);
		Read(ObjNetID);
		Read(ObjOwnerID);
		Read(SpawnTransform);
		
		if (Networking::GetObjectFromNetID(ObjNetID))
		{
			break;
		}
		
		for (auto& i : Objects::ObjectTypes)
		{
			if (i.ID == ObjTypeID)
			{
				Log::Print(StrUtil::Format("[Net]: Spawning replicated object %s with owner '%s'", 
					i.Name.c_str(),
					Networking::ClientIDToString(ObjOwnerID).c_str()),
					Log::LogColor::Gray);
			}
		}
		Objects::SpawnObjectFromID(ObjTypeID, SpawnTransform, ObjNetID)->NetOwner = ObjOwnerID;
	}
#endif
	break;
	case PacketType::ServerSceneTravel:
#if !SERVER
	{
	if (!Client::GetIsConnected())
	{
		return;
	}
	std::string Value;
	for (size_t i = 1; i < Data.size(); i++)
	{
		if (Data[i] == 0)
		{
			break;
		}
		Value.append({ (char)Data[i] });
	}

	Scene::LoadNewScene(Value);
	Scene::Tick();
	}
#endif
		break;
	case PacketType::NetworkEventTrigger:
		NetworkEvent::HandleNetworkEvent(this);
		break;
	case PacketType::NetworkEventAccept:
		NetworkEvent::HandleEventAccept(this);
		break;
	default:
		break;
	}
}

static void ReadPacket()
{

}
void Packet::AppendStringToData(std::string str)
{
	Data.reserve(Data.size() + str.size());
	
	for (char i : str)
	{
		Data.push_back((uint8_t)i);
	}
}

void Packet::Send(void* TargetAddr)
{
	IPaddress* Target = (IPaddress*)TargetAddr;
	Networking::SentPacket->address.host = Target->host;
	Networking::SentPacket->address.port = Target->port;
	Networking::SentPacket->len = (int)Data.size() + sizeof(uint64_t);
	
	memcpy(Networking::SentPacket->data + sizeof(uint64_t), Data.data(), Networking::SentPacket->len);
	memcpy(Networking::SentPacket->data, &PacketID, sizeof(uint64_t));
	PacketID++;
	int ret = SDLNet_UDP_Send(Networking::Socket, -1, Networking::SentPacket);

	if (!ret)
	{
		std::cout << "Error sending packet: " << SDLNet_GetError() << std::endl;
	}
}

void pktReceive()
{
	while (true)
	{
		int ret = SDLNet_CheckSockets(Networking::SocketSet, 150);
		if (ret == -1)
		{
			std::cout << SDLNet_GetError() << std::endl;
		}
		while (SDLNet_SocketReady(Networking::Socket))
		{
			if (SDLNet_UDP_Recv(Networking::Socket, threadPacket))
			{
				Packet NewP;

				if (threadPacket->len < 9)
				{
					continue;
				}

				NewP.Data.reserve(threadPacket->len);

				uint64_t pID = 0;

				memcpy(&pID, threadPacket->data, sizeof(pID));

				for (int i = sizeof(uint64_t); i < threadPacket->len; i++)
				{
					NewP.Data.push_back(threadPacket->data[i]);
				}
				NewP.FromAddr = &threadPacket->address;
				lines.insert(std::pair(pID, NewP));
			}
			else
			{
				break;
			}
		}
	}
}

std::map<uint64_t, Packet> Packet::Receive()
{
	std::unique_lock lock{ mutex };
	std::map<uint64_t, Packet> toProcess;
	if (cv.wait_for(lock, std::chrono::seconds(0), [&] { return !lines.empty(); }))
	{
		std::swap(lines, toProcess);
	}
#if 0
	for (auto& i : toProcess)
	{
		for (char c : i.second.Data)
		{
			std::hex(std::cout);
			std::cout << (int)c << " ";
			std::dec(std::cout);
		}
		std::cout << std::endl;
	}
#endif
	return toProcess;
}

void Packet::Init()
{
	if (pktThread)
	{
		return;
	}
	pktThread = new std::thread(pktReceive);
	threadPacket = SDLNet_AllocPacket(Packet::MAX_PACKET_SIZE);
	if (!threadPacket)
	{
		printf("Could not allocate packet\n");
		exit(2);
	}
}

void Packet::SetReceivePackets(bool NewRecv)
{
	RecvPacket = true;
}
#endif