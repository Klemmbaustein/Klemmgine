#include "NetworkSubsystem.h"
#include <Networking/Networking.h>

NetworkSubsystem::NetworkSubsystem()
{
	Name = "Network";
#if !EDITOR
	Networking::Init();
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
