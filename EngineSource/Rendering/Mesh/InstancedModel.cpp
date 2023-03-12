#include "InstancedModel.h"
#include <filesystem>
#include <fstream>
#include <Rendering/Utility/ShaderManager.h>
#include <Rendering/Mesh/ModelGenerator.h>
#include <Engine/Log.h>
#include <World/Graphics.h>
#include <World/Assets.h>
#include <World/Stats.h>
#include <GL/glew.h>


InstancedModel::InstancedModel(std::string Filename)
{
	int NumVertecies = 0;
	int NumIndices = 0;

	uint64_t NumMeshes = 0;

	if (std::filesystem::exists(Filename))
	{
		ModelGenerator::ModelData ModelData; ModelData.LoadModelFromFile(Filename);
		CastShadow = ModelData.CastShadow;
		TwoSided = ModelData.TwoSided;
		for (size_t i = 0; i < ModelData.Vertices.size(); i++)
		{
			InstancedMesh* NewMesh = new InstancedMesh(ModelData.Vertices[i], ModelData.Indices[i]);
			Meshes.push_back(NewMesh);
		}
		Materials = ModelData.Materials;
		if (ModelData.HasCollision)
		{
			NonScaledSize = ModelData.CollisionBox;
		}
		LoadMaterials(Materials);

		ConfigureVAO();
	}
}

void InstancedModel::LoadMaterials(std::vector<std::string> Materials)
{
	Materials.resize(Meshes.size());
	for (int j = 0; j < Meshes.size(); j++)
	{
		std::string m = Assets::GetAsset((Materials.at(j) + ".jsmat").substr(8));
		if (!std::filesystem::exists(m))
		{
			if (EngineDebug || IsInEditor)
				m = (Materials.at(j) + ".jsmat");
			else
				m = ("Assets/" + Materials.at(j) + ".jsmat");
			if (!std::filesystem::exists(m))
			{
				if (IsInEditor)
					Log::Print("Material given by the mesh does not exist. Falling back to default phong material - " + Materials[j], Vector3(1, 1, 0));
				m = "EditorContent/Materials/EngineDefaultPhong.jsmat";
			}
		}
		try
		{
			if (!std::filesystem::exists(m))
			{
				return;
			}

			Meshes.at(j)->MeshMaterial = Material::LoadMaterialFile(m, false);
			Meshes.at(j)->MeshShader = ReferenceShader("Shaders/" + Meshes.at(j)->MeshMaterial.VertexShader,
				"Shaders/" + Meshes.at(j)->MeshMaterial.FragmentShader);
			if (Meshes.at(j)->MeshShader == nullptr)
			{
				Meshes.at(j)->MeshShader = ReferenceShader("Shaders/basic.vert", "Shaders/basic.frag");
				Log::Print("Invalid Shader");
			}

		}
		catch (std::exception& e)
		{
			Log::Print(e.what(), Vector3(0.8f, 0.f, 0.f));
		}
		try
		{
			Meshes.at(j)->Uniforms.clear();
			for (int i = 0; i < Meshes[j]->MeshMaterial.Uniforms.size(); i++)
			{
				Meshes.at(j)->Uniforms.push_back(Uniform("", 0, nullptr));
				Meshes.at(j)->Uniforms.at(i).Name = Meshes[j]->MeshMaterial.Uniforms.at(i).UniformName;
				Meshes.at(j)->Uniforms.at(i).Type = Meshes[j]->MeshMaterial.Uniforms.at(i).Type;
				Meshes.at(j)->Uniforms.at(i).Content = (void*)Meshes[j]->MeshMaterial.Uniforms.at(i).Value.c_str();
			}
			Meshes.at(j)->ApplyUniforms();
		}
		catch (std::exception& e)
		{
			Log::Print(e.what());
		}
	}
}

InstancedModel::~InstancedModel()
{
	for (InstancedMesh* m : Meshes)
	{
		DereferenceShader("Shaders/" + m->MeshMaterial.VertexShader, "Shaders/" + m->MeshMaterial.FragmentShader);
		delete m;
	}
	Meshes.clear();
}
namespace CSM
{
	glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane);
	extern std::vector<unsigned int> ShadowMaps;
	const float cameraFarPlane = 700;
	extern std::vector<float> shadowCascadeLevels;
}

void InstancedModel::Render(Camera* WorldCamera, bool MainFrameBuffer, bool TransparencyPass)
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
		ModelView = WorldCamera->getView() * MatModel[0];
	}
	glm::mat4 InvModelView = glm::transpose(glm::inverse(ModelView));
	glBindBuffer(GL_ARRAY_BUFFER, MatBuffer);
	for (int i = 0; i < Meshes.size(); i++)
	{
		if (Meshes[i]->MeshMaterial.IsTranslucent != TransparencyPass) continue;
		Shader* CurrentShader = ((!Graphics::IsRenderingShadows || Meshes[i]->MeshMaterial.UseShadowCutout) ? Meshes.at(i)->MeshShader : Graphics::ShadowShader);
		CurrentShader->Bind();
		glUniformMatrix4fv(glGetUniformLocation(CurrentShader->GetShaderID(), "u_projection"), 1, GL_FALSE, &WorldCamera->GetProjection()[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(CurrentShader->GetShaderID(), "u_invmodelview"), 1, GL_FALSE, &InvModelView[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(CurrentShader->GetShaderID(), "u_viewpro"), 1, GL_FALSE, &WorldCamera->getViewProj()[0][0]);
		if (!Graphics::IsRenderingShadows)
			glUniformMatrix4fv(glGetUniformLocation(CurrentShader->GetShaderID(), "u_view"), 1, GL_FALSE, &WorldCamera->getView()[0][0]);
		Meshes.at(i)->Render(CurrentShader);
		Performance::DrawCalls++;
	}
}

size_t InstancedModel::AddInstance(Transform T)
{
	Instances.push_back(T);
	return Instances.size() - 1;
}

bool InstancedModel::RemoveInstance(size_t Inst)
{
	if (Inst > Instances.size()) return false;
	Instances.erase(Instances.begin() + Inst);
	return true;
}

void InstancedModel::ConfigureVAO()
{
	MatModel.clear();
	for (Transform &T : Instances)
	{
		glm::mat4 Inst = glm::mat4(1.f);

		Size = (NonScaledSize * T.Scale * 0.025f);
		Inst = glm::translate(Inst, (glm::vec3)T.Location);
		Inst = glm::rotate(Inst, T.Rotation.Y, glm::vec3(0, 1, 0));
		Inst = glm::rotate(Inst, T.Rotation.Z, glm::vec3(0, 0, 1));
		Inst = glm::rotate(Inst, T.Rotation.X, glm::vec3(1, 0, 0));

		MatModel.push_back(glm::scale(Inst, (glm::vec3)(T.Scale) * 0.025f));
	}
	if(MatBuffer != -1)
		glDeleteBuffers(1, &MatBuffer);
	glGenBuffers(1, &MatBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, MatBuffer);
	glBufferData(GL_ARRAY_BUFFER, MatModel.size() * sizeof(glm::mat4), &MatModel[0], GL_STATIC_DRAW);
	for (int i = 0; i < Meshes.size(); i++)
	{
		unsigned int VAO = Meshes[i]->MeshVertexBuffer->GetVAO();
		glBindVertexArray(VAO);
		// vertex attributes
		std::size_t vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);

		glBindVertexArray(0);
		Meshes[i]->SetInstances(Instances);
	}
}

void InstancedModel::SimpleRender(Shader* UsedShader)
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
	glBindBuffer(GL_ARRAY_BUFFER, MatBuffer);
	for (InstancedMesh* m : Meshes)
	{
		m->SimpleRender(UsedShader);
	}
	Performance::DrawCalls++;
}