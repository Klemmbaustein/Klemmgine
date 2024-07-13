#pragma once
#include <Engine/Subsystem/Subsystem.h>
#include <cstdint>

/**
 * @brief
 * Network subsystem.
 * 
 * Subsystem controlling the network functions of the engine.
 * @ingroup Subsystem
 */
class NetworkSubsystem : public Subsystem
{
public:
	/**
	 * @brief
	 * Creates the network subsystem.
	 * 
	 * @param DefaultServerPort
	 * The default server port. If the configuration isn't Server, connecting to any address will connect to this port by default.
	 * If the configuration is server, the server will be hosted on this port.
	 */
	NetworkSubsystem(uint16_t DefaultServerPort);
	~NetworkSubsystem();

	void Update() override;
};