#include "FrustumCulling.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/vec3.hpp>
namespace FrustumCulling
{
	bool Active = true;
	Frustum createFrustumFromCamera(const Camera& cam)
	{
		float aspect = 16.f/9.f;
		float fovY = 2.f;
		float zNear = NearPlane;
		float zFar = FarPlane;
		Frustum frustum;
		const float halfVSide = zFar * tanf(fovY * .5f);
		const float halfHSide = halfVSide * aspect;
		const glm::vec3 frontMultFar = zFar * cam.LookAt;

		frustum.nearFace = { cam.Position + zNear * cam.LookAt, cam.LookAt };
		frustum.farFace = { cam.Position + frontMultFar, Vector3() - cam.LookAt};
		frustum.rightFace = { cam.Position, Vector3::Cross(cam.Up, frontMultFar + cam.Right * halfHSide) };
		frustum.leftFace = { cam.Position, Vector3::Cross(frontMultFar - cam.Right * halfHSide, cam.Up) };
		frustum.topFace = { cam.Position, Vector3::Cross(cam.Right, frontMultFar - cam.Up * halfVSide) };
		frustum.bottomFace = { cam.Position, Vector3::Cross(frontMultFar + cam.Up * halfVSide, cam.Right) };
		return frustum;
	}
	Frustum CurrentCameraFrustum;
	bool AABB::isOnFrustum(const Frustum& camFrustum, const glm::vec3& transform, glm::vec3 scale) const
	{
		if (Active)
		{
			//Get global scale thanks to our transform
			const glm::vec3 globalCenter{ transform };

			// Scaled orientation
			const glm::vec3 right = glm::vec3(1, 0, 0) * scale;
			const glm::vec3 up = glm::vec3(0, 1, 0) * scale;
			const glm::vec3 forward = glm::vec3(0, 0, 1) * scale;

			const float newIi = std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, right)) +
				std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, up)) +
				std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, forward));

			const float newIj = std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, right)) +
				std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, up)) +
				std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, forward));

			const float newIk = std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, right)) +
				std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, up)) +
				std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, forward));

			//We not need to divise scale because it's based on the half extention of the AABB
			const AABB globalAABB(globalCenter, newIi, newIj, newIk);

			return (globalAABB.isOnOrForwardPlan(camFrustum.leftFace) &&
				globalAABB.isOnOrForwardPlan(camFrustum.rightFace) &&
				globalAABB.isOnOrForwardPlan(camFrustum.topFace) &&
				globalAABB.isOnOrForwardPlan(camFrustum.bottomFace) &&
				globalAABB.isOnOrForwardPlan(camFrustum.nearFace) &&
				globalAABB.isOnOrForwardPlan(camFrustum.farFace));
		}
		else return true;
	};
}