#include "Collision.h"
#include <algorithm>
#include <Math/Collision/TriangleIntersect.h>
#include <Objects/Components/CollisionComponent.h>
#include <Engine/Log.h>
#include <cmath>
#include <Math/Math.h>
#include <Math/Physics/Physics.h>

bool Collision::IsPointIn3DBox(Box a, Vector3 p)
{
	return (a.minX <= p.X && a.maxX >= p.X) &&
		(a.minY <= p.Y && a.maxY >= p.Y) &&
		(a.minZ <= p.Z && a.maxZ >= p.Z);
}
Collision::HitResponse Collision::LineTrace(Vector3 RayStart, Vector3 RayEnd, std::set<WorldObject*> ObjectsToIgnore, std::set<CollisionComponent*> MeshesToIgnore)
{
	HitResponse Result;

	auto Hit = Physics::RayCast(RayStart, RayEnd);

	Result.Hit = Hit.Hit;
	Result.HitComponent = Hit.HitComponent;
	Result.ImpactPoint = Hit.ImpactPoint;
	Result.Normal = Hit.Normal;
	Result.Distance = Hit.Distance;
	Result.Depth = Hit.Depth;
	if (Hit.HitComponent)
	{
		Result.HitObject = Hit.HitComponent->GetParent();
	}
	return Result;
}

//send help
Collision::HitResponse Collision::LineCheckForAABB(Collision::Box a, Vector3 RayStart, Vector3 RayEnd)
{
	Vector3 Normal;
	Vector3 RayDir = RayEnd - RayStart;
	float tmp;

	float txMin = (a.minX - RayStart.X) / RayDir.X;
	float txMax = (a.maxX - RayStart.X) / RayDir.X;
	if (txMax < txMin)
	{
		tmp = txMax;
		txMax = txMin;
		txMin = tmp;
	}
	float tyMin = (a.minY - RayStart.Y) / RayDir.Y;
	float tyMax = (a.maxY - RayStart.Y) / RayDir.Y;
	if (tyMax < tyMin)
	{
		tmp = tyMax;
		tyMax = tyMin;
		tyMin = tmp;
	}
	float tzMin = (a.minZ - RayStart.Z) / RayDir.Z;
	float tzMax = (a.maxZ - RayStart.Z) / RayDir.Z;
	if (tzMax < tzMin)
	{
		tmp = tzMax;
		tzMax = tzMin;
		tzMin = tmp;
	}
	float tMin;
	if (txMin > tyMin)
	{
		tMin = txMin;
		Normal = Vector3(-1, 0, 0);
	}
	else
	{
		tMin = tyMin;
		Normal = Vector3(0, -1, 0);
	}
	float tMax = (txMax < tyMax) ? txMax : tyMax;
	if (txMin > tyMax || tyMin > txMax) return HitResponse();
	if (tMin > tzMax || tzMin > tMax) return HitResponse();
	if (tzMin > tMin)
	{
		tMin = tzMin;
		Normal = Vector3(0, 0, -1);
	}
	if (tMax < tzMax) tMax = tzMax;
	Vector3 ImpactLocation = (RayDir * tMin);
	Vector3 RayDirNormalized = Vector3(RayDir.X > 0.0f ? 1.0f : -1.0f, RayDir.Y > 0.0f ? 1.0f : -1.0f, RayDir.Z > 0.0f ? 1.0f : -1.0f);
	Normal = Normal * RayDirNormalized;
	if (tMin < 0)
	{
		if (!IsPointIn3DBox(a, RayStart))
		{
			return HitResponse();
		}
	}
	return HitResponse(true, ImpactLocation + RayStart, Normal, tMin);
}

