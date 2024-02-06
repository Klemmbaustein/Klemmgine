#include "CollisionBox.h"
#include <iostream>
#include <cmath>

Collision::Box Collision::operator*(Box a, Vector3 b)
{
    return Box(a.minX * b.X, a.maxX * b.X, a.minY * b.Y, a.maxY * b.Y, a.minZ * b.Z, a.maxZ * b.Z);
}

Collision::Box Collision::operator+(Box a, Vector3 b)
{
    return Box(a.minX + b.X, a.maxX + b.X, a.minY + b.Y, a.maxY + b.Y, a.minZ + b.Z, a.maxZ + b.Z);
}

Vector3 Collision::Box::GetCenter()
{
	return Vector3(Vector3::Lerp(Vector3(minX, minY, minZ), Vector3(maxX, maxY, maxZ), 0.5));
}

Collision::Box::Box(Vector3 Triangle1, Vector3 Triangle2, Vector3 Triangle3)
{
	Vector3* TrianglePoints[3] =
	{
		&Triangle1,
		&Triangle2,
		&Triangle3
	};
	for (auto t : TrianglePoints)
	{
		if (t->X > maxX)
		{
			maxX = t->X;
		}
		if (t->Y > maxY)
		{
			maxY = t->Y;
		}
		if (t->Z > maxZ)
		{
			maxZ = t->Z;
		}
		if (t->X < minX)
		{
			minX = t->X;
		}
		if (t->Y < minY)
		{
			minY = t->Y;
		}
		if (t->Y < minZ)
		{
			minZ = t->Y;
		}
	}
}



Collision::Box Collision::Box::TransformBy(Transform Transform)
{
	Collision::Box OldBox = Box(minX, maxX, minY, maxY, minZ, maxZ) * Transform.Scale;
	Collision::Box RotatedBox = OldBox;
	float rot = std::abs(Vector3::Dot(Vector3::GetForwardVector(Transform.Rotation.RadiansToDegrees()), Vector3(0, 0, 1)));
	RotatedBox.maxX = std::lerp(OldBox.maxX, OldBox.maxZ, rot);
	RotatedBox.minX = std::lerp(OldBox.minX, OldBox.minZ, rot);
	RotatedBox.maxZ = std::lerp(OldBox.maxZ, OldBox.maxX, rot);
	RotatedBox.minZ = std::lerp(OldBox.minZ, OldBox.minX, rot);
	RotatedBox = RotatedBox + Transform.Location;
	return RotatedBox;
}

bool Collision::Box::IsOverlappingBox(const Box& Other)
{
	return (minX <= Other.maxX && maxX >= Other.minX) &&
		(minY <= Other.maxY && maxY >= Other.minY) &&
		(minZ <= Other.maxZ && maxZ >= Other.minZ);
}

bool Collision::Box::IsPointInBox(const Vector3& Other)
{
	return minX <= Other.X && maxX >= Other.X
		&& minY <= Other.Y && minX >= Other.Y
		&& maxZ <= Other.Z && maxZ >= Other.Z;
}

bool Collision::Box::SphereInBox(const Vector3& SpherePoint, float Radius)
{
	// get box closest point to sphere center by clamping
	float x = std::max(minX, std::min(SpherePoint.X, maxX));
	float y = std::max(minY, std::min(SpherePoint.Y, maxY));
	float z = std::max(minZ, std::min(SpherePoint.Z, maxZ));

	if (Vector3(x, y, z) == SpherePoint)
	{
		return true;
	}

	// this is the same as isPointInsideSphere
	float distance = std::sqrt(
		(x - SpherePoint.X) * (x - SpherePoint.X) +
		(y - SpherePoint.Y) * (y - SpherePoint.Y) +
		(z - SpherePoint.Z) * (z - SpherePoint.Z));

	return distance < Radius;
}

Vector3 Collision::Box::GetExtent()
{
	Vector3 center = GetCenter();
	return Vector3(maxX - center.X, maxY - center.Y, maxZ - center.Z);
}

float Collision::Box::GetLength()
{
	return sqrt(minX * minX + minY * minY + minZ * minZ + maxX * maxX + maxY * maxY + maxZ * maxZ);
}