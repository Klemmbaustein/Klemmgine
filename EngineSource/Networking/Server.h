#pragma once
#include <Engine/Application.h>
#if !EDITOR
#include "Packet.h"

class WorldObject;

namespace Server
{
	struct ClientInfo
	{
		uint64_t LastResponseTick = 0;
		void* IP = nullptr;
		uint64_t ID = 0;

		void SendClientSpawnRequest(int32_t ObjID, uint64_t NetID, uint64_t NetOwner, Transform SpawnTransform, std::string ObjProperties) const;
		void SendServerTravelRequest(std::string SceneName) const;
	};

	/**
	* @brief
	* Function called when a 'ConnectionRequest' packet has been received.
	* 
	* @param p 
	* The packet that caused the ConnectionRequest
	*/
	void OnConnectRequestReceived(Packet p);
	void DisconnectPlayer(void* IP);
	void DisconnectPlayer(uint64_t UID);
	void SpawnObject(int32_t ObjID, uint64_t NetID, Transform SpawnTransform, std::string ObjProperties);

	void HandleDestroyObject(WorldObject* o);

	void SetObjNetOwner(WorldObject* obj, uint64_t NetOwner);

	void Init();

	bool IsServer();

	void Update();
	void HandlePacket(Packet* p);

	void SendClientInfo(ClientInfo* c);

	ClientInfo* GetClientInfoFromIP(void* IP);
	ClientInfo* GetClientInfoFromID(uint64_t ID);

	const std::vector<ClientInfo>& GetClients();
}
#endif

namespace Server
{
	void AddOnPlayerConnectedCallback(void(*OnJoined)(uint64_t));
	void ClearOnPlayerConnectedCallbacks();

	void AddOnPlayerDisconnectedCallback(void(*OnLeft)(uint64_t));
	void ClearOnPlayerdisconnectedCallbacks();

}