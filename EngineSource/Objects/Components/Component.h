#pragma once
#include <Objects/WorldObject.h>

class ComponentSetter;

/**
* @defgroup Components
* @brief
* Components that can be attached to any WorldObject.
* 
* @ingroup Objects
*/

/**
* @brief
* A Component. Can be attached to any WorldObject.
* 
* @ingroup Components
*/
class Component
{
public:
	Component() {}
	virtual ~Component() {}

	virtual void Begin();
	virtual void Update();
	virtual void Destroy();
	/**
	* @brief
	* Gets the parent WorldObject.
	*/
	WorldObject* GetParent();
	friend class ComponentModifier;

	/**
	* @brief
	* Transform relative to the parent object.
	*/
	Transform RelativeTransform;

private:
	WorldObject* Parent = nullptr;
};

class ComponentModifier
{
public:
	static void SetParent(Component* c, WorldObject* p)
	{
		c->Parent = p;
	}
};