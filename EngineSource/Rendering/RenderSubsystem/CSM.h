#if !SERVER
#pragma once

#include "RenderSubsystem.h"
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <Rendering/Shader.h>

class Camera;

class CSM : public RenderSubsystem
{
public:
	static float CSMDistance;
	static const float cameraFarPlane;
	static std::vector<float> shadowCascadeLevels;
	static unsigned int LightFBO;
	static int Cascades;
	static unsigned int ShadowMaps;
	static unsigned int matricesUBO;

	static Shader* ShadowShader;

	static std::vector<glm::vec4> GetFrustumCornersWorldSpace(const glm::mat4& projview);

	static void UpdateMatricesUBO(Camera* From);
	static void BindLightSpaceMatricesToShader(const std::vector<glm::mat4>& Matrices, Shader* ShaderToBind);
	static std::vector<glm::vec4> GetFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);
	CSM();

	static void ReInit();

	static glm::mat4 GetLightSpaceMatrix(const float nearPlane, const float farPlane, Camera* From);

	static std::vector<glm::mat4> GetLightSpaceMatrices(Camera* From);
};
#endif