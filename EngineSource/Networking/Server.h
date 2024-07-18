#pragma once
#include <Engine/Application.h>
#include <functional>
#if !EDITOR
#include "Packet.h"

class SceneObject;

namespace Server
{
	struct ClientInfo
	{
		uint64_t LastResponseTick = 0;
		void* IP = nullptr;
		uint64_t ID = 0;
		// True if the client has accepted the most recent scene load request.
		bool LoadedInScene = false;

		void SendClientSpawnRequest(int32_t ObjID, uint64_t NetID, uint64_t NetOwner, Transform SpawnTransform, std::string ObjProperties) const;
		void SendServerTravelRequest(std::string SceneName);
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

	void HandleDestroyObject(SceneObject* o);

	void SetObjNetOwner(SceneObject* obj, uint64_t NetOwner);

	void Init();

	void ChangeScene(std::string NewSceneName);

	bool IsServer();

	void Update();
	void HandlePacket(Packet* p);

	void SendClientInfo(ClientInfo* c);
	ClientInfo* GetClientInfoFromIP(void* IP);
	ClientInfo* GetClientInfoFromID(uint64_t ID);

	const std::vector<ClientInfo>& GetClients();

	void OnClientAcceptSceneChange(uint64_t ClientID);
}
#endif

namespace Server
{
	void AddOnPlayerConnectedCallback(std::function<void(uint64_t PlayerID)> OnJoined);
	void ClearOnPlayerConnectedCallbacks();

	void AddOnPlayerDisconnectedCallback(std::function<void(uint64_t PlayerID)> OnLeft);
	void ClearOnPlayerDisconnectedCallbacks();

}