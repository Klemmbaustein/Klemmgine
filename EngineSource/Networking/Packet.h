#if !EDITOR
#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <cstring>

struct Packet
{
	static const int MAX_PACKET_SIZE;
	static uint64_t PacketID;

	std::vector<uint8_t> Data;
	void* FromAddr = nullptr;

	enum class PacketType
	{
		ConnectRequest = 0,
		ConnectionAccept = 1,
		DisconnectRequest = 2,
		ValueUpdate = 3,
		SpawnObject = 4,
		ServerSceneTravel = 5,
		NetworkEventTrigger = 6,
		NetworkEventAccept = 7,
	};


	void AppendStringToData(std::string str);

	void EvaluatePacket();
	void Send(void* TargetAddr);
	static std::map<uint64_t, Packet> Receive();
	static void Init();

	size_t StreamPos = 0;

	template<typename T>
	void Read(T& ReadData)
	{
		memcpy(&ReadData, &Data[StreamPos], sizeof(T));
		StreamPos += sizeof(T);
	}

	template<typename T>
	void Write(T WriteData)
	{
		Data.resize(Data.size() + sizeof(T));
		memcpy(&Data[Data.size() - sizeof(T)], &WriteData, sizeof(T));
	}

	void SetReceivePackets(bool NewRecv);
};
#endif