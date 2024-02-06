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
		/// t variable for LineTrace() function. ImpactPoint = Start + Direction * t.
		float t = INFINITY;
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
		HitResponse(bool Hit, Vector3 ImpactPoint, Vector3 Normal, float t = INFINITY, WorldObject* HitObject = nullptr)
		{
			this->Hit = Hit;
			this->ImpactPoint = ImpactPoint;
			this->t = t;
			this->Normal = Normal;
			this->HitObject = HitObject;
		}
	};
	struct CollisionMesh
	{
		CollisionMesh(std::vector<Vertex> Verts, std::vector<unsigned int> Indices, Transform T);
		CollisionMesh() { ModelMatrix = glm::mat4(1); }
		~CollisionMesh();
		glm::mat4 GetMatrix();
		void SetTransform(Transform T);
		Collision::HitResponse CheckAgainstMesh(CollisionMesh* b);
		Collision::HitResponse CheckAgainstLine(Vector3 RayStart, Vector3 RayEnd);
		Collision::HitResponse CheckAgainstAABB(const Box& b);
		bool CanOverlap = true;
		float SphereCollisionSize = 0.0f;
		Vector3 WorldPosition;
		Vector3 Scale = 1;
		Vector3 SpherePosition;
		HitResponse OverlapCheck(std::set<CollisionComponent*> MeshesToIgnore = {});
		std::vector<Vertex> Vertices;
		std::vector<Vertex>	RawVertices; std::vector<unsigned int> Indices;
	protected:
		float WorldScale = 1.0f;
		void ApplyMatrix();
		glm::mat4 ModelMatrix;

	};
	bool CollisionAABB(Box a, Box b);

	Collision::HitResponse TriangleLine(const Vector3& TriA, const Vector3& TriB, const Vector3& TriC, const Vector3& RayStart, const Vector3& RayEnd);

	HitResponse BoxOverlapCheck(Box a, Vector3 Offset);
	bool IsPointIn3DBox(Box a, Vector3 p);
	/**
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
	* 
	* @param MeshesToIgnore
	* Collision meshes that should be ignored in the collision check.
	*/
	HitResponse LineTrace(Vector3 RayStart, Vector3 RayEnd, std::set<WorldObject*> ObjectsToIgnore = {}, std::set<CollisionComponent*> MeshesToIgnore = {});

	HitResponse LineCheckForAABB(Box b, Vector3 RayStart, Vector3 RayEnd);
	extern std::vector<CollisionComponent*> CollisionBoxes;
}