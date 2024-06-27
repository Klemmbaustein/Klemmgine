#pragma once
#include <Engine/Subsystem/Subsystem.h>

class NetworkSubsystem : public Subsystem
{
public:
	NetworkSubsystem(uint16_t DefaultServerPort);
	~NetworkSubsystem();

	void Update() override;
};