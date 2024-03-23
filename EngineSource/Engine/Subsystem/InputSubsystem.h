#pragma once
#include "Subsystem.h"

class InputSubsystem : public Subsystem
{
public:
	InputSubsystem();
	void Update() override;

private:
	void PollInput();
};