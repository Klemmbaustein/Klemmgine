#pragma once
#include <cstdint>

class SceneObject;

#if !EDITOR
#include <string>
#include <Math/Vector.h>


namespace Networking
{
	void Init(uint16_t DefaultPort);
	void HandleTick();
	void ReceivePackets();
	void Update();
	SceneObject* SpawnReplicatedObjectFromID(uint32_t ID, Transform Position);
	std::string IPtoStr(void* addr);
	void DisconnectPlayer(void* IP);
	void Exit();
	bool IPEqual(void* ip1, void* ip2);
	uint32_t GetTickRate();
	uint64_t GetGameTick();

	bool GetIsServerTickFrame();
	float GetTickDelta();

	std::string ClientIDToString(uint64_t ID);

	uint16_t GetDefaultPort();
}
#endif

namespace Networking
{
	const uint64_t ServerID = UINT64_MAX;
	SceneObject* GetObjectFromNetID(uint64_t NetID);
}
