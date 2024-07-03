#include "Client.h"
#if !EDITOR
#include "NetworkingInternal.h"
#include <Engine/Log.h>
#include "Packet.h"
#include "Networking.h"
#include <Engine/Utility/StringUtility.h>
#include <Objects/SceneObject.h>
#include <Engine/Application.h>

namespace Client
{
	IPaddress ConnectedServer;
	bool IsConnected = false;
	bool IsConnecting = false;
	uint64_t ClientID = 0;
	Application::Timer ConnectionTimer;
	static int ConnectionAttempts = 0;
	static const int MAX_CONNECTION_ATTEMPTS = 3;

	void AttemptConnection()
	{
		Packet InitPacket;
		InitPacket.Data =
		{
			(uint8_t)Packet::PacketType::ConnectRequest,
		};
		InitPacket.Send(&ConnectedServer);
	}

	struct UnhandledValueUpdate
	{
		uint64_t ObjNetID = 0;
		std::string Name;
		std::string Value;
	};

	std::vector<UnhandledValueUpdate> UnhandledValueUpdates;
}

void Client::ConnectToServer(std::string Address, uint16_t Port)
{
#if SERVER
	return;
#endif
	IPaddress ip;
	int ret = SDLNet_ResolveHost(&ip, Address.c_str(), Port);
	if (ret == -1)
	{
		Log::Print("Failed to resolve host: " + Address);
		return;
	}
	if (IsConnected)
	{
		return;
	}
	IsConnecting = true;
	IsConnected = false;

	ConnectedServer = ip;
	Networking::Socket = Networking::InitSocketFrom(&ip);
	Packet::Init();
	ConnectionTimer.Reset();
	ConnectionAttempts = 0;
	AttemptConnection();
}

void Client::Disconnect()
{
	if (IsConnected)
	{
		Packet p;
		p.Data =
		{
			(uint8_t)Packet::PacketType::DisconnectRequest
		};
		p.Send(&ConnectedServer);
	}
	IsConnected = false;
	IsConnecting = false;
	Log::Print("Disconnected from server.", Log::LogColor::Blue);
}

void Client::OnConnected(Packet* p)
{
	if (IsConnected)
	{
		return;
	}

	if (p->Data.size() < sizeof(uint64_t) + 1)
	{
		return;
	}

	ConnectedServer = *(IPaddress*)p->FromAddr;
	IsConnecting = false;
	memcpy(&ClientID, &p->Data[1], sizeof(uint64_t));
	IsConnected = true;
	Log::PrintMultiLine(StrUtil::Format("Connected to server: %s\nClientUID: %i",
			Networking::IPtoStr(p->FromAddr),
			(int)ClientID
		),
		Log::LogColor::Green,
		"[Net]: ");
}

void Client::Update()
{
	if (IsConnecting && ConnectionTimer.Get() > 2)
	{
		if (ConnectionAttempts >= MAX_CONNECTION_ATTEMPTS)
		{
			IsConnecting = false;
			Log::Print("[Net]: Failed to connect. Max connection retries reached.", Log::LogColor::Yellow);
			return;
		}
		ConnectionAttempts++;
		Log::Print(StrUtil::Format("[Net]: Connection timed out - retrying (%i/%i)",
			ConnectionAttempts,
			MAX_CONNECTION_ATTEMPTS),
			Log::LogColor::Yellow);
		AttemptConnection();
		ConnectionTimer.Reset();
	}

	for (size_t i = 0; i < UnhandledValueUpdates.size(); i++)
	{
		auto& vu = UnhandledValueUpdates[i];
		if (HandleValueUpdate(vu.ObjNetID, vu.Name, vu.Value, false))
		{
			UnhandledValueUpdates.erase(UnhandledValueUpdates.begin() + i);
			i--;
		}
	}

	if (!IsConnected)
	{
		return;
	}

	for (SceneObject* i : Objects::AllObjects)
	{
		if (i->NetOwner == GetClientID())
		{
			Networking::SendObjectInfo(i, &ConnectedServer);
		}
	}
}

uint64_t Client::GetClientID()
{
#if SERVER
	return Networking::ServerID;
#else
	return ClientID;
#endif
}

bool Client::GetIsConnecting()
{
	return IsConnecting;
}

bool Client::GetIsConnected()
{
#if !SERVER
	return IsConnected;
#else
	return true;
#endif
}

bool Client::HandleValueUpdate(uint64_t ObjNetID, std::string Name, std::string Value, bool Force)
{
	auto obj = Networking::GetObjectFromNetID(ObjNetID);

	if (obj == nullptr)
	{
		if (!Force)
		{
			return false;
		}
		UnhandledValueUpdate u;
		u.Name = Name;
		u.ObjNetID = ObjNetID;
		u.Value = Value;

		UnhandledValueUpdates.push_back(u);
		return false;
	}

	if (Name == "_owner")
	{
		obj->NetOwner = std::stoull(Value);
		return true;
	}

	if (Name == "_pos")
	{
		obj->GetTransform().Position = Vector3::FromString(Value);
		return true;
	}

	if (Name == "_rot")
	{
		obj->GetTransform().Rotation = Vector3::FromString(Value);
		return true;
	}

	if (Name == "_scl")
	{
		obj->GetTransform().Scale = Vector3::FromString(Value);
		return true;
	}
	try
	{
		for (auto& i : obj->Properties)
		{
			if (i.PType != SceneObject::Property::PropertyType::NetProperty)
			{
				continue;
			}

			if (i.Name != Name)
			{
				continue;
			}

			switch (i.NativeType)
			{
			case NativeType::Int:
				*(int*)i.Data = std::stoi(Value);
				break;
			case NativeType::Float:
				*(float*)i.Data = std::stof(Value);
				break;
			case NativeType::Vector3:
			case NativeType::Vector3Color:
			case NativeType::Vector3Rotation:
				*(Vector3*)i.Data = Vector3::FromString(Value);
				break;
			case NativeType::Bool:
				*(bool*)i.Data = std::stoi(Value);
				break;
			default:
				break;
			}
		}
	}
	catch (std::exception)
	{

	}
	return true;
}

void* Client::GetCurrentServerAddr()
{
	return &ConnectedServer;
}

void Client::Exit()
{
	if (IsConnected)
	{
		Disconnect();
	}
}

#else

bool Client::GetIsConnected()
{
	return false;
}

size_t Client::GetClientID()
{
	return 0;
}

#endif