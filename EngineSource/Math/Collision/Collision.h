#pragma once
#include "Math/Vector.h"
#include "Rendering/Mesh/Model.h"
#include "CollisionBox.h"
#include "Objects/WorldObject.h"
#include <set>


class CollisionComponent;

/**
* @file
* @brief
* File containing functions and variables for Collision.
*/

/**
* @brief
* Namespace containing functions related to Collision
* 
* @deprecated New collision/physics related functions are in Physics.h.
*/
namespace Collision
{
	/**
	* @brief
	* Describes the result of a collision check.
	*/
	struct HitResponse
	{
		/// True if the collision check hit something, false if not.
		bool Hit = false;
		/// The point where the collision check hit something.
		Vector3 ImpactPoint;
		/// The depth of the collision.
		float Depth = 0;
		/// The distance the collision ray has traveled, from 0 - 1. If the collision is not a RayCast or ShapeCast, this can be ignored.
		float Distance = INFINITY;
		/// The object that was hit.
		WorldObject* HitObject = nullptr;
		/// The component that was hit.
		Component* HitComponent = nullptr;
		/// The normal vector of the collision.
		Vector3 Normal;
		HitResponse()
		{
			Hit = false;
		}
		HitResponse(bool Hit, Vector3 ImpactPoint, Vector3 Normal, float Distance = INFINITY, WorldObject* HitObject = nullptr)
		{
			this->Hit = Hit;
			this->ImpactPoint = ImpactPoint;
			this->Distance = Distance;
			this->Normal = Normal;
			this->HitObject = HitObject;
		}
	};

	bool IsPointIn3DBox(Box a, Vector3 p);
	/**
	* 
	* @deprecated Use Physics::RayCast instead.
	* 
	* @brief
	* Traces a line from RayStart to RayEnd.
	* 
	* @param RayStart
	* Where the ray should start.
	* 
	* @param RayEnd
	* Where the ray should end.
	* 
	* @param ObjectsToIgnore
	* Objects that should be ignored in the collision check.
	*/
	HitResponse LineTrace(Vector3 RayStart, Vector3 RayEnd, std::set<WorldObject*> ObjectsToIgnore = {});

	HitResponse LineCheckForAABB(Box b, Vector3 RayStart, Vector3 RayEnd);
}