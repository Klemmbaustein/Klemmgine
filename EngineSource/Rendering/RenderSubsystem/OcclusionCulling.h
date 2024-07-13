#pragma once
#if !SERVER
#include "RenderSubsystem.h"
#include <cstdint>
#include <stack>

class Model;
class FramebufferObject;
struct Shader;

/**
* @brief
* Frustum culling subsystem.
* 
* @ingroup Subsystem
* @ingroup Internal
*/
class OcclusionCulling : public RenderSubsystem
{
public:

	/// True if occlusion culling is active, false if not.
	bool Active = true;

	OcclusionCulling();

	Shader* CullShader;

	bool RenderOccluded(Model* m, size_t i, FramebufferObject* Buffer);
	void OcclusionCheck(Model* m, size_t i, FramebufferObject* Buffer);

	void UpdateOcclusionStatus(Model* m);

	void CheckQueries(FramebufferObject* Buffer);

	void FreeOcclusionQuery(uint8_t Index);

private:
	Model* OcclusionCullMesh = nullptr;
	uint8_t NumOcclusionQueries = 0;
	unsigned int OcclusionQueries[256];
	bool QueriesActive[256];
	std::stack<Model*> CulledModels;
};

#endif
