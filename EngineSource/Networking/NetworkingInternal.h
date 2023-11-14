#pragma once
#include <SDL_net.h>
#include <cstdint>

class WorldObject;
class WorldObject;

namespace Networking
{
	extern SDLNet_SocketSet SocketSet;

	extern UDPsocket Socket;
	extern UDPpacket* SentPacket;
	extern float TickTimer;
	extern size_t Gametick;
	UDPsocket InitSocketFrom(IPaddress* Target);

	void SendObjectInfo(WorldObject* obj, void* TargetAddr);
}