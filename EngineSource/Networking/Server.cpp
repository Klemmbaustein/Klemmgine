#include "Server.h"
#include <vector>

namespace Server
{
	std::vector<std::function<void(uint64_t)>> OnConnectedCallbacks;
	std::vector<std::function<void(uint64_t)>> OnDisconnectedCallbacks;
}

void Server::AddOnPlayerConnectedCallback(std::function<void(uint64_t PlayerID)> OnJoined)
{
	OnConnectedCallbacks.push_back(OnJoined);
}
void Server::ClearOnPlayerConnectedCallbacks()
{
	OnConnectedCallbacks.clear();
}

void Server::AddOnPlayerDisconnectedCallback(std::function<void(uint64_t PlayerID)> OnLeft)
{
	OnDisconnectedCallbacks.push_back(OnLeft);
}

void Server::ClearOnPlayerDisconnectedCallbacks()
{
	OnDisconnectedCallbacks.clear();
}

#if !EDITOR
#include <Engine/Log.h>
#include <Engine/Utility/StringUtility.h>
#include <Networking/NetworkingInternal.h>
#include "Networking.h"
#include <Objects/SceneObject.h>
#include <Engine/Subsystem/Console.h>
#include <Engine/Subsystem/Scene.h>
#include <Engine/Utility/FileUtility.h>
#include "NetworkEvent.h"
#include <Engine/Stats.h>

namespace Server
{
	static bool ShouldQuitOnPlayerDisconnect = false;
	static std::vector<ClientInfo> Clients;
	static uint64_t UIDCounter = 0;
	static bool LoadingNewScene = false;
	namespace TPS
	{
		static bool PrintTPS = false;
		static uint16_t NewTicksPerSecond = 0;
		static uint16_t TicksPerSecond = 0;
		static Application::Timer PerSecondTimer;

		static void PrintTickStats()
		{
			Log::Print("[Net]: Time: "
				+ std::to_string((int)Stats::Time)
				+ "s. Ticks per second: "
				+ std::to_string(TPS::TicksPerSecond)
				+ ". Target: "
				+ std::to_string(Networking::GetTickRate())
				+ ". Delta time: "
				+ std::to_string(1000.0f * Stats::DeltaTime)
				+ "ms", Log::LogColor::Gray);
		}
	}

	static ClientInfo* ServerClient = new ClientInfo();

	void ClientInfo::SendClientSpawnRequest(int32_t ObjID, uint64_t NetID, uint64_t NetOwner, Transform SpawnTransform, std::string ObjProperties) const
	{
		Packet p;
		p.Write((uint8_t)Packet::PacketType::SpawnObject);
		p.Write(ObjID);
		p.Write(NetID);
		p.Write(NetOwner);
		p.Write(SpawnTransform);
		p.AppendStringToData(ObjProperties);
		p.Send(IP);

	}
	void ClientInfo::SendServerTravelRequest(std::string SceneName)
	{
		LoadedInScene = false;
		NetworkEvent::TriggerNetworkEvent("__scene", { SceneName }, nullptr, ID);
	}

	static void HandleClientDisconnect(ClientInfo* Client)
	{
		for (auto& i : OnDisconnectedCallbacks)
		{
			i(Client->ID);
		}

		for (size_t i = 0; i < Clients.size(); i++)
		{
			if (Clients[i].ID == Client->ID)
			{
				Clients.erase(Clients.begin() + i);
			}
		}
		NetworkEvent::ClearEventsFor(Client->ID);

		if (ShouldQuitOnPlayerDisconnect && Clients.empty())
		{
			Application::Quit();
		}
	}
}

void Server::OnConnectRequestReceived(Packet p)
{
	auto Info = GetClientInfoFromIP(p.FromAddr);

	Packet ReturnPacket;
	ReturnPacket.Data =
	{
		(uint8_t)Packet::PacketType::ConnectionAccept,
	};

	if (Info)
	{
		ReturnPacket.Write(Info->ID);
	}
	else
	{
		ReturnPacket.Write(UIDCounter);
	}
	auto NewIP = new IPaddress();
	NewIP->host = ((IPaddress*)p.FromAddr)->host;
	NewIP->port = ((IPaddress*)p.FromAddr)->port;
	ClientInfo NewClient;
	NewClient.ID = UIDCounter;
	NewClient.IP = NewIP;
	NewClient.LastResponseTick = Networking::GameTick;

	ReturnPacket.Send(NewClient.IP);

	NewClient.SendServerTravelRequest(FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene));

	if (Info)
	{
		return;
	}
	UIDCounter++;
	Clients.push_back(NewClient);
	Log::PrintMultiLine(StrUtil::Format("Client connected to server:\n\tip: %s\n\tuid: %s",
		Networking::IPtoStr(NewClient.IP).c_str(),
		std::to_string(NewClient.ID).c_str()
	),
		Log::LogColor::Blue,
		"[Net]: ");
}

void Server::DisconnectPlayer(void* IP)
{
	for (size_t i = 0; i < Clients.size(); i++)
	{
		if (Networking::IPEqual(Clients[i].IP, IP))
		{
			Log::PrintMultiLine(StrUtil::Format("Client %i has disconnected.",
				Clients[i].ID), Log::LogColor::Yellow, "[Net]: ");
			Packet p;
			p.Data =
			{
				(uint8_t)Packet::PacketType::DisconnectRequest
			};
			p.Send(Clients[i].IP);
			HandleClientDisconnect(&Clients[i]);
			break;
		}
	}
}

void Server::DisconnectPlayer(uint64_t UID)
{
	for (auto& i : Clients)
	{
		if (i.ID == UID)
		{
			Packet p;
			p.Data =
			{
				(uint8_t)Packet::PacketType::DisconnectRequest
			};
			p.Send(i.IP);
			DisconnectPlayer(i.IP);
			return;
		}
	}
	Log::Print("[Net]: Could not disconnect player with uid " + std::to_string(UID) + " because that uid does not exist.");
}

void Server::SpawnObject(int32_t ObjID, uint64_t NetID, Transform SpawnTransform, std::string ObjProperties)
{
	for (auto& i : Clients)
	{
		// The client will receive a list of all replicated objects once it finishes loading into the scene.
		if (i.LoadedInScene)
		{
			i.SendClientSpawnRequest(ObjID, NetID, Networking::ServerID, SpawnTransform, ObjProperties);
		}
	}
}

void Server::HandleDestroyObject(SceneObject* o)
{
	for (auto& i : Server::Clients)
	{
		NetworkEvent::TriggerNetworkEvent("__destr", {}, o, i.ID);
	}
}

void Server::SetObjNetOwner(SceneObject* obj, uint64_t NetOwner)
{
	Packet p;
	p.Data =
	{
		(uint8_t)Packet::PacketType::ValueUpdate,
	};

	p.Data.resize(9);
	memcpy(&p.Data[1], &obj->NetID, sizeof(size_t));

	p.AppendStringToData("_owner=" + std::to_string(NetOwner));
	for (auto& i : Clients)
	{
		if (i.LoadedInScene)
		{
			p.Send(i.IP);
		}
	}
	obj->NetOwner = NetOwner;
}

void Server::Init()
{
	Console::ConsoleSystem->RegisterCommand(Console::Command("playerlist", []() {
		std::string PlayerList;
		for (auto& i : Clients)
		{
			PlayerList.append("\tuid: " + std::to_string(i.ID)
				+ ", ip: " + Networking::IPtoStr(i.IP)
				+ ", last response: " + std::to_string(Networking::GameTick - i.LastResponseTick)
				+ " ticks\n");
		}
		Log::PrintMultiLine(StrUtil::Format("Players connected: (%i):\n%s", Clients.size(), PlayerList.c_str()), Log::LogColor::White, "[Net]: ");
		}, {}));

	Console::ConsoleSystem->RegisterCommand(Console::Command("quitondisconnect", []() {
		ShouldQuitOnPlayerDisconnect = true;
		}, {}));

	Console::ConsoleSystem->RegisterCommand(Console::Command("tickrate", []() {
		TPS::PrintTickStats();
		}, { }));

	Console::ConsoleSystem->RegisterCommand(Console::Command("serverperf", []() {
		TPS::PrintTPS = !TPS::PrintTPS;
		if (TPS::PrintTPS)
		{
			Log::Print("[Net]: Printing performance stats... 'serverperf' to disable.");
		}
		}, { }));
}

void Server::ChangeScene(std::string NewSceneName)
{
	Log::Print(StrUtil::Format("[Net]: Server: Changing active scene to '%s'", NewSceneName.c_str()));
	Scene::LoadNewScene(NewSceneName);
	for (auto& i : Clients)
	{
		i.SendServerTravelRequest(NewSceneName);
	}
}

bool Server::IsServer()
{
#ifdef SERVER
	return true;
#else
	return false;
#endif
}

void Server::Update()
{
	TPS::NewTicksPerSecond++;
	if (TPS::PerSecondTimer.Get() > 1.0f)
	{
		TPS::PerSecondTimer.Reset();
		TPS::TicksPerSecond = TPS::NewTicksPerSecond;
		TPS::NewTicksPerSecond = 0;
		if (TPS::PrintTPS)
		{
			TPS::PrintTickStats();
		}
	}

	for (size_t i = 0; i < Clients.size(); i++)
	{
		SendClientInfo(&Clients[i]);

		// Give the client extra time before timing out if they haven't loaded into the scene yet.
		// To make sure they didn't time out while still loading in.
		size_t TimeoutTicks = (size_t)Networking::GetTickRate() * 5 * (Clients[i].LoadedInScene ? 1 : 5);

		if (Networking::GameTick - Clients[i].LastResponseTick > TimeoutTicks)
		{
			Log::PrintMultiLine(StrUtil::Format("Client %i has timed out.\n\tLast seen tick: %i\n\tCurrent tick %i",
				Clients[i].ID,
				(int)Clients[i].LastResponseTick,
				(int)Networking::GameTick),
				Log::LogColor::Yellow, "[Net]: ");
			HandleClientDisconnect(&Clients[i]);
			break;
		}
	}
}

void Server::HandlePacket(Packet* p)
{
	ClientInfo* PacketSender = GetClientInfoFromIP(p->FromAddr);

	if (PacketSender)
	{
		PacketSender->LastResponseTick = Networking::GameTick;
	}
}

void Server::SendClientInfo(ClientInfo* c)
{
	for (SceneObject* i : Objects::AllObjects)
	{
		Networking::SendObjectInfo(i, c->IP);
	}
}

Server::ClientInfo* Server::GetClientInfoFromIP(void* IP)
{
	for (auto& i : Clients)
	{
		if (Networking::IPEqual(i.IP, IP))
		{
			return &i;
		}
	}
	return nullptr;
}

Server::ClientInfo* Server::GetClientInfoFromID(uint64_t ID)
{
	for (auto& i : Clients)
	{
		if (i.ID == ID)
		{
			return &i;
		}
	}
	return nullptr;
}

const std::vector<Server::ClientInfo>& Server::GetClients()
{
	return Clients;
}
void Server::OnClientAcceptSceneChange(uint64_t ClientID)
{
	ClientInfo* Client = GetClientInfoFromID(ClientID);

	if (!Client)
	{
		Log::Print(StrUtil::Format("[Net]: %s: Invalid client ID: %s", __FUNCTION__, std::to_string(ClientID).c_str()));
		return;
	}

	if (Client->LoadedInScene)
	{
		return;
	}

	Client->LoadedInScene = true;

	for (SceneObject* i : Objects::AllObjects)
	{
		if (i->GetIsReplicated())
		{
			Client->SendClientSpawnRequest(i->GetObjectDescription().ID, i->NetID, i->NetOwner, i->GetTransform(), i->GetPropertiesAsString());
		}
	}

	for (std::function<void(uint64_t)>& i : OnConnectedCallbacks)
	{
		i(ClientID);
	}
}
#endif