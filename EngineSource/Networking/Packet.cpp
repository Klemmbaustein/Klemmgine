#if !EDITOR
#include "Packet.h"
#include <Engine/Log.h>
#include "NetworkEvent.h"
#include "NetworkingInternal.h"
#include <iostream>
#include <Networking/Server.h>
#include <Networking/Client.h>
#include <Engine/Utility/StringUtility.h>
#include <Engine/Subsystem/Scene.h>
#include <thread>
#include <condition_variable>
#include <mutex>
#include "Networking.h"

const int Packet::MAX_PACKET_SIZE = 512;
uint64_t Packet::PacketID = 0;
namespace pkt
{
	std::thread* pktThread = nullptr;
	static bool RecvPacket = false;
	std::condition_variable cv;
	std::mutex PacketReceiveMutex;
	std::map<uint64_t, Packet> Lines;
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

		auto Values = StrUtil::SeparateString(ReadString(), ';');

		for (auto& i : Values)
		{
			size_t Equals = i.find_first_of("=");
			if (Equals == std::string::npos)
			{
				break;
			}
			Client::HandleValueUpdate(ObjID,
				i.substr(0, Equals),
				i.substr(Equals + 1), true);
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
				Log::Print(StrUtil::Format("[Net]: Spawning replicated object %s with owner '%s' (NetID: %i)", 
					i.Name.c_str(),
					Networking::ClientIDToString(ObjOwnerID).c_str(),
					(int)ObjNetID),
					Log::LogColor::Gray);
			}
		}
		auto obj = Objects::SpawnObjectFromID(ObjTypeID, SpawnTransform, ObjNetID);
		obj->NetOwner = ObjOwnerID;
		obj->LoadProperties(ReadString());
		obj->OnPropertySet();
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
	Networking::PacketData->address.host = Target->host;
	Networking::PacketData->address.port = Target->port;
	Networking::PacketData->len = (int)Data.size() + sizeof(uint64_t);
	
	memcpy(Networking::PacketData->data + sizeof(uint64_t), Data.data(), Networking::PacketData->len);
	memcpy(Networking::PacketData->data, &PacketID, sizeof(uint64_t));
	PacketID++;
	int Result = SDLNet_UDP_Send(Networking::Socket, -1, Networking::PacketData);

	if (!Result)
	{
		Log::Print(StrUtil::Format("Error sending packet: %s", SDLNet_GetError()));
		Log::Print(StrUtil::Format("WSAGetLastError: %i", WSAGetLastError()));
	}
}

void PacketReceive()
{
	while (true)
	{
		if (!Networking::SocketSet)
		{
			return;
		}
		int ret = SDLNet_CheckSockets(Networking::SocketSet, 150);
		if (ret == -1)
		{
			Log::Print(StrUtil::Format("Error reading sockets: %s", SDLNet_GetError()));
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
				std::lock_guard lock{ PacketReceiveMutex };
				Lines.insert(std::pair(pID, NewP));
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
	std::lock_guard lock{ PacketReceiveMutex };
	std::map<uint64_t, Packet> toProcess;
	std::swap(Lines, toProcess);
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
	pktThread = new std::thread(PacketReceive);
	threadPacket = SDLNet_AllocPacket(Packet::MAX_PACKET_SIZE);
	if (!threadPacket)
	{
		printf("Could not allocate packet\n");
		exit(2);
	}
}

std::string Packet::ReadString()
{
	std::string Value;
	while (StreamPos < Data.size())
	{
		if (Data[StreamPos] == 0)
		{
			break;
		}
		Value.push_back((char)Data[StreamPos]);
		StreamPos++;
	}
	return Value;
}

void Packet::SetReceivePackets(bool NewRecv)
{
	RecvPacket = true;
}

#endif