#pragma once
#include "Math/Vector.h"
#include <algorithm>
namespace Collision
{
	struct Box
	{
		float minX = 0;
		float maxX = 0;
		float minY = 0;
		float maxY = 0;
		float minZ = 0;
		float maxZ = 0;

		Vector3 GetCenter();

		Box(Vector3 Triangle1, Vector3 Triangle2, Vector3 Triangle3);

		Box(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
		{
			this->minX = minX;
			this->minY = minY;
			this->minZ = minZ;
			this->maxX = maxX;
			this->maxY = maxY;
			this->maxZ = maxZ;
		}
		Box()
		{

		}
		Box TransformBy(Transform Transform);

		bool IsOverlappingBox(const Box& Other);

		bool IsPointInBox(const Vector3& Other);

		bool SphereInBox(const Vector3& SpherePoint, float Radius);

		Vector3 GetExtent();

		float GetLength();
	};
	Box operator*(Box a, Vector3 b);
	Box operator+(Box a, Vector3 b);
}