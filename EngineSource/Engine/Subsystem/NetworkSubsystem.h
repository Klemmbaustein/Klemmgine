#pragma once
#include <Engine/Subsystem/Subsystem.h>

class NetworkSubsystem : public Subsystem
{
public:
	NetworkSubsystem();
	~NetworkSubsystem();

	void Update() override;
};