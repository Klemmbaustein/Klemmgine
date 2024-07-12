#if !SERVER
#include "OcclusionCulling.h"
#include <GL/glew.h>
#include <Rendering/Framebuffer.h>
#include <Rendering/Mesh/Model.h>
#include <Engine/Application.h>
#include <Rendering/ShaderManager.h>


OcclusionCulling::OcclusionCulling()
	: RenderSubsystem(true)
{
	Name = "Occlude";

	ModelGenerator::ModelData CullMeshData;
	CullMeshData.AddElement().MakeCube(2, 0);
	OcclusionCullMesh = new Model(CullMeshData);
	OcclusionCullMesh->TwoSided = true;
	CullShader = ShaderManager::ReferenceShader("Shaders/Internal/cull.vert", "Shaders/Internal/cull.frag");

	memset(QueriesActive, 0, sizeof(QueriesActive));
	glGenQueries(256, OcclusionQueries);
}

bool OcclusionCulling::RenderOccluded(Model* m, size_t i, FramebufferObject* Buffer)
{
	GLuint sampleCount = 0;
	if (!m || !m->Visible)
	{
		return false;
	}

	if ((m->Size.extents * m->ModelTransform.Scale).Length() > 500.0f
		|| (!m->IsOcclusionCulled && i != Stats::FrameCount % Buffer->Renderables.size())
		|| !m->ShouldCull)
	{
		m->Render(Buffer->FramebufferCamera, Buffer == Graphics::MainFramebuffer, false);
		m->IsOcclusionCulled = false;
		return true;
	}

	if (m->RunningQuery)
	{
		if (!m->IsOcclusionCulled)
		{
			m->Render(Buffer->FramebufferCamera, Buffer == Graphics::MainFramebuffer, false);
		}
		return !m->IsOcclusionCulled;
	}
	if (m->IsOcclusionCulled)
	{
		CulledModels.push(m);
		return false;
	}

	GLuint Query = 0;
	for (uint8_t i = 0; i < 255; i++)
	{
		if (!QueriesActive[i])
		{
			QueriesActive[i] = true;
			m->OcclusionQueryIndex = i;
			Query = OcclusionQueries[i];
			break;
		}
	}

	if (Query == 0)
	{
		if (!m->IsOcclusionCulled)
		{
			m->Render(Buffer->FramebufferCamera, Buffer == Graphics::MainFramebuffer, false);
		}
		return !m->IsOcclusionCulled;
	}

	glBeginQuery(GL_ANY_SAMPLES_PASSED, Query);
	m->Render(Buffer->FramebufferCamera, Buffer == Graphics::MainFramebuffer, false);
	glEndQuery(GL_ANY_SAMPLES_PASSED);
	m->RunningQuery = true;
	return true;
}

void OcclusionCulling::OcclusionCheck(Model* m, size_t i, FramebufferObject* Buffer)
{
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	GLuint Query = 0;
	for (uint8_t i = 0; i < 255; i++)
	{
		if (!QueriesActive[i])
		{
			QueriesActive[i] = true;
			m->OcclusionQueryIndex = i;
			Query = OcclusionQueries[i];
			break;
		}
	}

	if (Query == 0)
	{
		if (!m->IsOcclusionCulled)
		{
			m->Render(Buffer->FramebufferCamera, Buffer == Graphics::MainFramebuffer, false);
		}
		return;
	}

	glBeginQuery(GL_ANY_SAMPLES_PASSED, Query);
	Transform t = Transform(
		m->ModelTransform.Position + Vector3::RotateVector(m->Size.center, m->ModelTransform.Rotation),
		m->ModelTransform.Rotation,
		m->Size.extents * m->ModelTransform.Scale
	);
	OcclusionCullMesh->ModelTransform = t;
	OcclusionCullMesh->UpdateTransform();
	OcclusionCullMesh->SimpleRender(CullShader);

	glEndQuery(GL_ANY_SAMPLES_PASSED);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	m->RunningQuery = true;
}

void OcclusionCulling::FreeOcclusionQuery(uint8_t Index)
{
#if !SERVER
	QueriesActive[Index] = false;
#endif
}

void OcclusionCulling::UpdateOcclusionStatus(Model* m)
{
	if (!m->RunningQuery)
	{
		return;
	}
	GLuint Query = OcclusionCulling::OcclusionQueries[m->OcclusionQueryIndex];
	GLuint QueryVal = 0;
	glGetQueryObjectuiv(Query, GL_QUERY_RESULT_AVAILABLE, &QueryVal);
	if (!QueryVal)
	{
		return;
	}
	glGetQueryObjectuiv(Query, GL_QUERY_RESULT, &QueryVal);
	m->IsOcclusionCulled = QueryVal == 0 || Graphics::IsWireframe;
	m->RunningQuery = false;
	OcclusionCulling::QueriesActive[m->OcclusionQueryIndex] = false;

}

void OcclusionCulling::CheckQueries(FramebufferObject* Buffer)
{
	while (!OcclusionCulling::CulledModels.empty())
	{
		OcclusionCulling::OcclusionCheck(CulledModels.top(), 0, Buffer);
		OcclusionCulling::CulledModels.pop();
	}
}

#endif