#pragma once
#include <cstdint>
#if !EDITOR
#include <string>
#include <Math/Vector.h>

class WorldObject;

namespace Networking
{
	void Init();
	void HandleTick();
	void ReceivePackets();
	void Update();
	WorldObject* SpawnReplicatedObjectFromID(uint32_t ID, Transform Location);
	std::string IPtoStr(void* addr);
	void DisconnectPlayer(void* IP);
	void Exit();
	bool IPEqual(void* ip1, void* ip2);
	uint32_t GetTickRate();
	uint64_t GetGameTick();

	std::string ClientIDToString(uint64_t ID);

	WorldObject* GetObjectFromNetID(uint64_t NetID);
}
#endif

namespace Networking
{
	const uint64_t ServerID = UINT64_MAX;
}

namespace Project
{
	extern bool UseNetworkFunctions;
}