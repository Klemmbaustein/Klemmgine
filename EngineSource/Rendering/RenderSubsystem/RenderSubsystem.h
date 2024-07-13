#pragma once
#include <Engine/Subsystem/Subsystem.h>

/**
* @brief
* A the parent class of all subsystems related to rendering.
*/
class RenderSubsystem : public Subsystem
{
public:
	/**
	* @brief
	* Constructs the subsystem.
	* 
	* @param IsDerived
	* True if the subsystem is derived from RenderSubsystem, false if it isn't.
	*/
	RenderSubsystem(bool IsDerived = false);
	~RenderSubsystem();

	/**
	* @brief
	* Function called when the render resolution is changed.
	*/
	virtual void OnRendererResized();

	static void ResizeAll();
};