#pragma once
#include <string>
#include <cstdint>

struct Packet;

namespace Client
{
	void ConnectToServer(std::string Address, uint16_t Port);
	void Disconnect();

	void OnConnected(Packet* p);

	void Update();
	void Exit();

	uint64_t GetClientID();

	bool HandleValueUpdate(uint64_t ObjNetID, std::string Name, std::string Value, bool Force);

	bool GetIsConnecting();
	bool GetIsConnected();

	void* GetCurrentServerAddr();
}