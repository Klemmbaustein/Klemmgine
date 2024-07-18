#if !EDITOR
#include "NetworkEvent.h"
#include "Packet.h"
#include <Engine/Log.h>
#include "Client.h"
#include "Networking.h"
#include "Server.h"
#include <unordered_set>
#include <Objects/SceneObject.h>
#include <Engine/Utility/StringUtility.h>
#include <Engine/Subsystem/Scene.h>

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
	static void* CallingClient = nullptr;
	
	static void SendEvent(const Event& e)
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
			auto Client = Server::GetClientInfoFromID(e.TargetClient);
			if (Client)
			{
				IP = Client->IP;
			}
			else
			{
				return;
			}
		}
		p.Send(IP);
	}
}

void NetworkEvent::TriggerNetworkEvent(std::string Name, std::vector<std::string> Arguments, SceneObject* Target, uint64_t TargetClient)
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

	if (Data->Data.size() < sizeof(EventID) + sizeof(ObjID) + 1)
	{
		return;
	}

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
	std::vector<std::string> Values = StrUtil::SeparateString(Value, ';');
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

	if (ReceivedEvents.contains(EventID))
	{
		return;
	}

	ReceivedEvents.insert(EventID);

#if !SERVER
	if (ObjID == UINT64_MAX)
	{
		if (Name == "__scene" && Values.size() == 1)
		{
			Scene::LoadNewScene(Values.at(0), true);
		}
		return;
	}
#endif

	SceneObject* Target = Networking::GetObjectFromNetID(ObjID);
	if (!Target)
	{
		// TODO: Handle missing objects by requesting it from the server
		Log::Print("[Net]: Unknown object: " + std::to_string(ObjID), Log::LogColor::Yellow);
		return;
	}

#if SERVER
	if (Server::GetClientInfoFromIP(Data->FromAddr)->ID == Target->NetID)
	{
		return;
	}
#endif

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
#if SERVER
		if (i.NativeType != SceneObject::NetEvent::EventType::Server)
		{
			continue;
		}
#endif
		if (i.Name == Name)
		{
			CallingClient = Data->FromAddr;
			(Target->*i.Function)(Values);
			CallingClient = nullptr;
			break;
		}
	}
}

void NetworkEvent::HandleEventAccept(Packet* Data)
{
	if (Data->Data.size() < sizeof(EventID) + 1)
	{
		return;
	}

	uint64_t EventID = 0;
	Data->Read(EventID);

	for (size_t i = 0; i < SentEvents.size(); i++)
	{
		if (SentEvents[i].EventID != EventID)
		{
			continue;
		}
#if SERVER
		auto& EventName = SentEvents[i].Name;
		size_t FirstSemicolon = EventName.find_first_of(";");
		if (EventName.substr(0, FirstSemicolon) == "__scene" && SentEvents[i].Object == UINT64_MAX)
		{
			Server::OnClientAcceptSceneChange(SentEvents[i].TargetClient);
		}
#endif

		SentEvents.erase(SentEvents.begin() + i);
		break;
	}
}

void* NetworkEvent::GetCallingClient()
{
	return CallingClient;
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
void NetworkEvent::ClearEventsFor(uint64_t PlayerID)
{
	for (size_t i = 0; i < SentEvents.size(); i++)
	{
		if (SentEvents[i].TargetClient == PlayerID)
		{
			SentEvents.erase(SentEvents.begin() + i);
			i--;
		}
	}
}
#endif