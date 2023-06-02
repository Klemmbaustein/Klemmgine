#pragma once
#include "Math/Vector.h"
#include "Rendering/Mesh/Model.h"
#include "CollisionBox.h"
#include "Objects/WorldObject.h"
#include <set>


class CollisionComponent;

namespace Collision
{

	struct HitResponse
	{
		bool Hit;
		Vector3 ImpactPoint;
		float t = INFINITY;
		WorldObject* HitObject = nullptr;
		CollisionComponent* HitComponent = nullptr;
		Vector3 Normal;
		HitResponse()
		{
			Hit = false;
			ImpactPoint = Vector3();
		}
		HitResponse(bool Hit, Vector3 ImpactPoint, Vector3 Normal, float t = INFINITY, WorldObject* InModel = nullptr)
		{
			this->Hit = Hit;
			this->ImpactPoint = ImpactPoint;
			this->t = t;
			this->Normal = Normal;
			HitObject = InModel;
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
		float SphereCollisionSize = 0;
		Vector3 WorldPosition;
		Vector3 SpherePosition;
		HitResponse OverlapCheck(std::set<CollisionComponent*> MeshesToIgnore = {});
		std::vector<Vertex> Vertices;
		std::vector<Vertex>	RawVertices; std::vector<unsigned int> Indices;
	protected:
		float WorldScale = 1;
		void ApplyMatrix();
		glm::mat4 ModelMatrix;

	};
	bool CollisionAABB(Box a, Box b);


	HitResponse BoxOverlapCheck(Box a, Vector3 Offset);
	bool IsPointIn3DBox(Box a, Vector3 p);
	HitResponse LineTrace(Vector3 RayStart, Vector3 RayEnd, std::set<WorldObject*> ObjectsToIgnore = {}, std::set<CollisionComponent*> MeshesToIgnore = {});

	HitResponse LineCheckForAABB(Box b, Vector3 RayStart, Vector3 RayEnd);
	extern std::vector<CollisionComponent*> CollisionBoxes;
}