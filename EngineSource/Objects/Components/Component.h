#pragma once
#include <Objects/SceneObject.h>

class ComponentSetter;

/**
* @defgroup Components
* @brief
* Components that can be attached to any SceneObject.
* 
* @ingroup Objects
*/

/**
* @brief
* A Component. Can be attached to any SceneObject.
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
	* Gets the parent SceneObject.
	*/
	SceneObject* GetParent();
	friend class ComponentModifier;

	/**
	* @brief
	* Transform relative to the parent object.
	*/
	Transform RelativeTransform;

	virtual Transform GetWorldTransform();

private:
	SceneObject* Parent = nullptr;
};

class ComponentModifier
{
public:
	static void SetParent(Component* c, SceneObject* p)
	{
		c->Parent = p;
	}
};