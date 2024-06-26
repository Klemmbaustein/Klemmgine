#include "NetworkSubsystem.h"
#include <Networking/Networking.h>

NetworkSubsystem::NetworkSubsystem(uint16_t DefaultServerPort)
{
	Name = "Network";
#if !EDITOR
	Networking::Init(DefaultServerPort);
#endif
}

NetworkSubsystem::~NetworkSubsystem()
{
#if !EDITOR
	Networking::Exit();
#endif
}

void NetworkSubsystem::Update()
{
#if !EDITOR
	Networking::Update();
#endif
}
