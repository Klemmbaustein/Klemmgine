#if !EDITOR
#include "NetworkEvent.h"
#include "Packet.h"
#include <Engine/Log.h>
#include "Client.h"
#include "Networking.h"
#include "Server.h"
#include <unordered_set>
#include <Objects/WorldObject.h>

namespace NetworkEvent
{
	uint64_t EventID = 0;
	struct Event
	{
		std::string Name;
		uint64_t Object = 0;
		uint64_t LastTick = 0;
		uint64_t EventID = 0;
		uint64_t TargetClient = Networking::ServerID;
	};

	std::unordered_set<uint64_t> ReceivedEvents;

	std::vector<Event> SentEvents;
	
	void SendEvent(const Event& e)
	{
		Packet p;
		p.Write((uint8_t)Packet::PacketType::NetworkEventTrigger);
		p.Write(e.EventID);
		p.Write(e.Object);
		p.AppendStringToData(e.Name);
		void* IP = nullptr;
		if (e.TargetClient == Networking::ServerID)
		{
			IP = Client::GetCurrentServerAddr();
		}
		else
		{
			IP = Server::GetClientInfoFromID(e.TargetClient)->IP;
		}
		p.Send(IP);
	}
}

void NetworkEvent::TriggerNetworkEvent(std::string Name, std::vector<std::string> Arguments, WorldObject* Target, uint64_t TargetClient)
{
	Event e;
	e.EventID = EventID++;
	e.LastTick = Networking::GetGameTick();
	e.Name = Name;
	for (auto& i : Arguments)
	{
		e.Name.append(";" + i);
	}
	e.TargetClient = TargetClient;
	if (Target)
	{
		e.Object = Target->NetID;
	}
	else
	{
		e.Object = UINT64_MAX;
	}
	SentEvents.push_back(e);
	SendEvent(e);
}

void NetworkEvent::HandleNetworkEvent(Packet* Data)
{
	uint64_t EventID = 0;
	uint64_t ObjID = 0;
	Data->Read(EventID);
	Data->Read(ObjID);

	std::string Value;
	for (size_t i = 1 + sizeof(uint64_t) + sizeof(uint64_t); i < Data->Data.size(); i++)
	{
		if (Data->Data[i] == 0)
		{
			break;
		}
		Value.append({ (char)Data->Data[i] });
	}
	std::vector<std::string> Values;
	size_t Last = 0;
	do
	{
		size_t PrevLast = Last;
		Last = Value.find_first_of(";", PrevLast) + 1;
		Values.push_back(Value.substr(PrevLast, Last - 1));
	} while (Last != std::string::npos + 1);

	if (Values.empty())
	{
		return;
	}
	std::string Name = Values[0];
	Values.erase(Values.begin());

	Packet p;
	p.Write((uint8_t)Packet::PacketType::NetworkEventAccept);
	p.Write(EventID);
	p.Send(Data->FromAddr);
	if (!ReceivedEvents.contains(EventID))
	{
		ReceivedEvents.insert(EventID);

		WorldObject* Target = Networking::GetObjectFromNetID(ObjID);
		if (!Target)
		{
			// TODO: Handle missing objects by requesting it from the server
			return;
		}
#if !SERVER
		if (Networking::IPEqual(Client::GetCurrentServerAddr(), Data->FromAddr))
		{
			if (Name == "__destr")
			{
				Objects::DestroyObject(Target);
				return;
			}
		}
#endif

		for (const auto& i : Target->NetEvents)
		{
			if (i.Name == Name)
			{
				(Target->*i.Function)(Values);
			}
		}
	}
}

void NetworkEvent::HandleEventAccept(Packet* Data)
{
	uint64_t EventID = 0;
	Data->Read(EventID);
	for (size_t i = 0; i < SentEvents.size(); i++)
	{
		if (SentEvents[i].EventID == EventID)
		{
			SentEvents.erase(SentEvents.begin() + i);
			break;
		}
	}
}

void NetworkEvent::Update()
{
#if !SERVER
	if (!Client::GetIsConnected())
	{
		SentEvents.clear();
	}
#endif
	for (size_t i = 0; i < SentEvents.size(); i++)
	{
		if (Networking::GetGameTick() - SentEvents[i].LastTick > 8)
		{
			SentEvents[i].LastTick = Networking::GetGameTick();
			SendEvent(SentEvents[i]);
		}
	}
}
#endif