#pragma once
#include "Camera.h"
#include <Math/Collision/CollisionBox.h>

namespace FrustumCulling
{
	extern bool Active;
	struct Plan
	{
		Vector3 normal = { 0.f, 1.f, 0.f }; // unit vector
		float     distance = 0.f;        // Distance with origin

		Plan() = default;

		Plan(const Vector3& p1, const Vector3& norm)
			: normal(norm.Normalize()),
			distance(Vector3::Dot(normal, p1))
		{}

		float getSignedDistanceToPlan(const Vector3& point) const
		{
			return Vector3::Dot(normal, point) - distance;
		}
	};

	struct Frustum
	{
		Plan topFace;
		Plan bottomFace;

		Plan rightFace;
		Plan leftFace;

		Plan farFace;
		Plan nearFace;
	};
	struct Volume
	{
		virtual bool isOnFrustum(const Frustum& camFrustum,
			const glm::vec3& transform, glm::vec3 scale) const = 0;
	};
	struct AABB : public Volume
	{
		glm::vec3 center{ 0.f, 0.f, 0.f };
		glm::vec3 extents{ 0.f, 0.f, 0.f };

		AABB(const glm::vec3& min, const glm::vec3& max)
			: Volume{},
			center{ (max + min) * 0.5f },
			extents{ max.x - center.x, max.y - center.y, max.z - center.z }
		{}

		AABB(const glm::vec3& inCenter, float iI, float iJ, float iK)
			: Volume{}, center{ inCenter }, extents{ iI, iJ, iK }
		{}

		AABB(Collision::Box b)
		{
			glm::vec3 min = glm::vec3(b.minX, b.minY, b.minZ);
			glm::vec3 max = glm::vec3(b.maxX, b.maxY, b.maxZ);
			center = (max + min) * 0.5f;
			extents = glm::vec3(max.x - center.x, max.y - center.y, max.z - center.z);
		}

		AABB()
		{

		}

		bool isOnOrForwardPlan(const Plan& plan) const
		{
			// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
			const float r = extents.x * std::abs(plan.normal.X) +
				extents.y * std::abs(plan.normal.Y) + extents.z * std::abs(plan.normal.Z);

			return -r <= plan.getSignedDistanceToPlan(center);
		}


		bool isOnFrustum(const Frustum& camFrustum, const glm::vec3& transform, glm::vec3 scale) const;

	};

	Frustum createFrustumFromCamera(const Camera& cam);

	extern Frustum CurrentCameraFrustum;
}