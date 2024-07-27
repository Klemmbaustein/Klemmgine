#include "NetworkSubsystem.h"
#include <Networking/Networking.h>
#include <Networking/Server.h>
#include <Networking/Client.h>

std::string NetworkSubsystem::ConnectTarget;
uint16_t NetworkSubsystem::ConnectTargetPort = 0;

NetworkSubsystem::NetworkSubsystem(uint16_t DefaultServerPort)
{
	Name = "Network";
#if !EDITOR
	Networking::Init(DefaultServerPort);

	if (!ConnectTarget.empty())
	{
		if (ConnectTargetPort == 0)
		{
			ConnectTargetPort = DefaultServerPort;
		}
		Client::ConnectToServer(ConnectTarget, ConnectTargetPort);
	}
#endif
}

NetworkSubsystem::~NetworkSubsystem()
{
#if !EDITOR
	Networking::Exit();
#endif
}

bool NetworkSubsystem::IsActive()
{
	return Subsystem::GetSubsystemByName("Network");
}

void NetworkSubsystem::QuitOnDisconnect()
{
#if !EDITOR
	Server::ShouldQuitOnPlayerDisconnect = true;
#endif
}

void NetworkSubsystem::Connect(std::string Address, uint16_t Port)
{
#if !EDITOR
	if (IsActive())
	{
		Client::ConnectToServer(Address, Port);
	}
	else
	{
		ConnectTarget = Address;
		ConnectTargetPort = Port;
	}
#endif
}

void NetworkSubsystem::Connect(std::string Address)
{
#if !EDITOR
	Connect(Address, Networking::GetDefaultPort());
#endif
}

void NetworkSubsystem::Update()
{
#if !EDITOR
	Networking::Update();
#endif
}
