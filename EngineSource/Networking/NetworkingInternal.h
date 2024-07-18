#pragma once
#include <SDL_net.h>
#include <cstdint>

class SceneObject;
class SceneObject;

namespace Networking
{
	extern SDLNet_SocketSet SocketSet;

	extern UDPsocket Socket;
	extern UDPpacket* PacketData;
	extern float TickTimer;
	extern size_t GameTick;
	UDPsocket InitSocketFrom(IPaddress* Target);

	void SendObjectInfo(SceneObject* obj, void* TargetAddr);
}