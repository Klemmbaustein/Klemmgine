#if !SERVER
#include "Model.h"
#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include "Rendering/Camera/Camera.h"
#include <filesystem>
#include <Rendering/Utility/ShaderManager.h>
#include <Engine/Log.h>
#include <Rendering/Graphics.h>
#include <Engine/Stats.h>
#include <Engine/File/Assets.h>
#include <GL/glew.h>
#include <Rendering/Texture/Material.h>
#include <Rendering/Mesh/Mesh.h>
#include <Engine/Application.h>

const extern bool IsInEditor;
const extern bool EngineDebug;


Model::Model(std::string Filename)
{
	if (std::filesystem::exists(Filename))
	{
		ModelMeshData.LoadModelFromFile(Filename);
		CastShadow = ModelMeshData.CastShadow;
		TwoSided = ModelMeshData.TwoSided;
		size_t NumVerts = 0;
		for (size_t i = 0; i < ModelMeshData.Elements.size(); i++)
		{
			Mesh* NewMesh = new Mesh(ModelMeshData.Elements[i].Vertices, ModelMeshData.Elements[i].Indices, 
				Material::LoadMaterialFile(ModelMeshData.Elements[i].ElemMaterial));
			Meshes.push_back(NewMesh);
		}
		ShouldCull = NumVerts > 100;
		HasCollision = ModelMeshData.HasCollision;
		NonScaledSize = ModelMeshData.CollisionBox.GetLength();
		Vector3 Extent = ModelMeshData.CollisionBox.GetExtent();
		Size = FrustumCulling::AABB(ModelMeshData.CollisionBox.GetCenter() * 0.025f, Extent.X, Extent.Y, Extent.Z);
		ConfigureVAO();
	}
	else
	{
		Log::Print("Model does not exist: " + Filename);
	}
}

Model::Model(ModelGenerator::ModelData Data)
{
	ModelMeshData = Data;
	CastShadow = Data.CastShadow;
	TwoSided = Data.TwoSided;
	size_t NumVerts = 0;
	for (auto& i : Data.Elements)
	{
		Material mat;
		if (!i.ElemMaterial.empty())
		{
			mat = Material::LoadMaterialFile(i.ElemMaterial);
		}
		Mesh* NewMesh = new Mesh(i.Vertices, i.Indices, mat);
		NumVerts += i.Vertices.size();
		Meshes.push_back(NewMesh);
	}
	ShouldCull = NumVerts > 100;
	HasCollision = Data.HasCollision;
	NonScaledSize = Data.CollisionBox.GetLength();
	Vector3 Extent = Data.CollisionBox.GetExtent() * 0.025f;
	Size = FrustumCulling::AABB(Data.CollisionBox.GetCenter() * 0.025f, Extent.X, Extent.Y, Extent.Z);
	ConfigureVAO();
}


Model::~Model()
{
	for (Mesh* m : Meshes)
	{
		delete m;
	}
	glDeleteBuffers(1, &MatBuffer);
	if (RunningQuery)
	{
		Application::FreeOcclusionQuery(OcclusionQueryIndex);
	}
}
namespace CSM
{
	glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane);
	extern std::vector<unsigned int> ShadowMaps;
	const float cameraFarPlane = 700;
	extern std::vector<float> shadowCascadeLevels;
}

void Model::Render(Camera* WorldCamera, bool MainFrameBuffer, bool TransparencyPass)
{
	if (Visible
		&& (Size.isOnFrustum(FrustumCulling::CurrentCameraFrustum, ModelTransform.Location, ModelTransform.Scale * NonScaledSize * 0.02f)
			|| !MainFrameBuffer))
	{
		if (TwoSided)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
		}
		glm::mat4 ModelView;
		if (!Graphics::IsRenderingShadows)
		{
			ModelViewProjection = WorldCamera->getViewProj() * MatModel;
			ModelView = WorldCamera->getView() * MatModel;
		}
		glm::mat4 InvModelView = glm::transpose(glm::inverse(ModelView));
		glBindBuffer(GL_ARRAY_BUFFER, MatBuffer);

		for (int i = 0; i < Meshes.size(); i++)
		{
			if (Meshes[i]->RenderContext.Mat.IsTranslucent != TransparencyPass) continue;
			Shader* CurrentShader = Meshes[i]->RenderContext.GetShader();
			CurrentShader->Bind();
			glUniformMatrix4fv(glGetUniformLocation(CurrentShader->GetShaderID(), "u_projection"), 1, GL_FALSE, &WorldCamera->GetProjection()[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(CurrentShader->GetShaderID(), "u_invmodelview"), 1, GL_FALSE, &InvModelView[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(CurrentShader->GetShaderID(), "u_viewpro"), 1, GL_FALSE, &WorldCamera->getViewProj()[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(CurrentShader->GetShaderID(), "u_view"), 1, GL_FALSE, &WorldCamera->getView()[0][0]);
			Meshes.at(i)->Render(CurrentShader, MainFrameBuffer);
			Performance::DrawCalls++;
		}
	}
}

void Model::ConfigureVAO()
{
	if (MatBuffer != -1)
		glDeleteBuffers(1, &MatBuffer);
	glGenBuffers(1, &MatBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, MatBuffer);
	glBufferData(GL_ARRAY_BUFFER, 1 * sizeof(glm::mat4), &MatModel, GL_STATIC_DRAW);
	for (int i = 0; i < Meshes.size(); i++)
	{
		unsigned int VAO = Meshes[i]->MeshVertexBuffer->VAO;
		glBindVertexArray(VAO);
		// vertex attributes
		std::size_t vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (void*)0);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (void*)(1 * vec4Size));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (void*)(2 * vec4Size));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (void*)(3 * vec4Size));

		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);

		glBindVertexArray(0);
	}
}

void Model::SimpleRender(Shader* UsedShader)
{
	if (!Visible) return;
	if (Size.isOnFrustum(FrustumCulling::CurrentCameraFrustum, ModelTransform.Location, ModelTransform.Scale * NonScaledSize * 0.025f))
	{
		UsedShader->Bind();
		if (TwoSided)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
		}
		glUniformMatrix4fv(glGetUniformLocation(UsedShader->GetShaderID(), "u_model"), 1, GL_FALSE, &MatModel[0][0]);
		for (Mesh* m : Meshes)
		{
			m->SimpleRender(UsedShader);
			Performance::DrawCalls++;
		}
	}
}

void Model::SetUniform(Material::Param NewUniform, uint8_t MeshIndex)
{
	Meshes[MeshIndex]->SetUniform(NewUniform);
}
#endif