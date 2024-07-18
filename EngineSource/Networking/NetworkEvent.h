#pragma once
#include <string>
#include <vector>

class SceneObject;
struct Packet;

namespace NetworkEvent
{
	void TriggerNetworkEvent(std::string Name, std::vector<std::string> Arguments, SceneObject* Target, uint64_t TargetClient);

	void HandleNetworkEvent(Packet* Data);
	void HandleEventAccept(Packet* Data);

	/**
	* @brief
	* When a NetEvent is called, this function will return a pointer to the IP of client that called the event.
	*/
	void* GetCallingClient();

	void Update();

	void ClearEventsFor(uint64_t PlayerID);
}