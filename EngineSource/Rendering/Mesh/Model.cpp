#include "Model.h"
#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include "Rendering/Camera/Camera.h"
#include <filesystem>
#include <Rendering/Utility/ShaderManager.h>
#include <Engine/Log.h>
#include <UI/EditorUI/MaterialFunctions.h>
#include <World/Graphics.h>
#include <World/Stats.h>
#include <World/Assets.h>
#include <GL/glew.h>
#include <Rendering/Texture/Material.h>
#include <Rendering/Mesh/Mesh.h>

const extern bool IsInEditor;
const extern bool EngineDebug;

class ModelException : public std::exception
{
public:
	ModelException(std::string ErrorType)
	{
		Exception = "Model loading error thrown: " + ErrorType;
	}

	virtual const char* what() const throw()
	{
		return Exception.c_str();
	}

	std::string Exception;
};

Model::Model(std::string Filename)
{

	uint64_t NumMeshes = 0;

	if (std::filesystem::exists(Filename))
	{
		ModelMeshData.LoadModelFromFile(Filename);
		CastShadow = ModelMeshData.CastShadow;
		TwoSided = ModelMeshData.TwoSided;
		for (size_t i = 0; i < ModelMeshData.Vertices.size(); i++)
		{
			Mesh* NewMesh = new Mesh(ModelMeshData.Vertices[i], ModelMeshData.Indices[i]);
			Meshes.push_back(NewMesh);
		}
		Materials = ModelMeshData.Materials;
		HasCollision = ModelMeshData.HasCollision;
		NonScaledSize = ModelMeshData.CollisionBox.GetLength();
		LoadMaterials(Materials);
		ConfigureVAO();
	}
	else
	{
		Log::Print("Model does not exist!");
	}
}

Model::Model(ModelGenerator::ModelData Data)
{
	ModelMeshData = Data;
	CastShadow = Data.CastShadow;
	TwoSided = Data.TwoSided;
	for (size_t i = 0; i < Data.Vertices.size(); i++)
	{
		Mesh* NewMesh = new Mesh(Data.Vertices.at(i), Data.Indices.at(i));
		Meshes.push_back(NewMesh);
	}
	Materials = Data.Materials;
	HasCollision = Data.HasCollision;
	NonScaledSize = Data.CollisionBox.GetLength();
	LoadMaterials(Materials);
	ConfigureVAO();
}


Model::~Model()
{
	for (Mesh* m : Meshes)
	{
		DereferenceShader("Shaders/" + m->MeshMaterial.VertexShader, "Shaders/" + m->MeshMaterial.FragmentShader);
		delete m;
	}
	glDeleteBuffers(1, &MatBuffer);
	Meshes.clear();
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
		&& (Size.isOnFrustum(FrustumCulling::CurrentCameraFrustum, ModelTransform.Location, ModelTransform.Scale * NonScaledSize * 0.02)
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
			if (Meshes[i]->MeshMaterial.IsTranslucent != TransparencyPass) continue;
			Shader* CurrentShader = ((!Graphics::IsRenderingShadows || Meshes[i]->MeshMaterial.UseShadowCutout) ? Meshes.at(i)->MeshShader : Graphics::ShadowShader);
			CurrentShader->Bind();
			glUniformMatrix4fv(glGetUniformLocation(CurrentShader->GetShaderID(), "u_projection"), 1, GL_FALSE, &WorldCamera->GetProjection()[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(CurrentShader->GetShaderID(), "u_invmodelview"), 1, GL_FALSE, &InvModelView[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(CurrentShader->GetShaderID(), "u_viewpro"), 1, GL_FALSE, &WorldCamera->getViewProj()[0][0]);
			if (!Graphics::IsRenderingShadows)
				glUniformMatrix4fv(glGetUniformLocation(CurrentShader->GetShaderID(), "u_view"), 1, GL_FALSE, &WorldCamera->getView()[0][0]);
			Meshes.at(i)->Render(CurrentShader, MainFrameBuffer);
			Performance::DrawCalls++;
		}
	}
}

void Model::LoadMaterials(std::vector<std::string> Materials)
{
	Materials.resize(Meshes.size());
	for (int j = 0; j < Meshes.size(); j++)
	{
		std::string m;
		if (!Materials.at(j).empty())
		{
			m = Assets::GetAsset((Materials.at(j) + ".jsmat").substr(8));
			if (!std::filesystem::exists(m))
			{
				m = Assets::GetAsset((Materials.at(j) + ".jsmat"));
#if EDITOR || DEBUG
				if (!std::filesystem::exists(m))
				{
					m = Materials[j] + ".jsmat";
				}
#endif
				if (!std::filesystem::exists(m))
				{
					Log::Print("Material given by the mesh does not exist. Falling back to default phong material - " + Materials[j], Vector3(1, 1, 0));
					m = "EditorContent/Materials/EngineDefaultPhong.jsmat";
#ifdef RELEASE
					throw ModelException("Model has invalid material assigned: \"" + Materials[j] + "\". Cannot fall back to default material");
#endif
				}
			}
			if (std::filesystem::is_empty(m))
			{
				Log::Print("Material given by the mesh is empty. Falling back to default phong material - " + Materials[j], Vector3(1, 1, 0));
				m = "EditorContent/Materials/EngineDefaultPhong.jsmat";
			}
		}
		else
		{
			Log::Print("Material given by the mesh does not exist. Falling back to default phong material - " + Materials[j], Vector3(1, 1, 0));
			m = "EditorContent/Materials/EngineDefaultPhong.jsmat";
		}
		try
		{
			if (!std::filesystem::exists(m))
			{
				return;
			}

			Material mat = Material::LoadMaterialFile(m, false);
			Meshes.at(j)->MeshMaterial = mat;
			Meshes.at(j)->MeshShader = ReferenceShader("Shaders/" + mat.VertexShader, "Shaders/" + mat.FragmentShader);
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
			for (int i = 0; i < Meshes.at(j)->MeshMaterial.Uniforms.size(); i++)
			{
				Meshes.at(j)->Uniforms.push_back(Uniform("", 0, nullptr));
				Meshes.at(j)->Uniforms.at(i).Name = Meshes.at(j)->MeshMaterial.Uniforms.at(i).UniformName;
				Meshes.at(j)->Uniforms.at(i).Type = Meshes.at(j)->MeshMaterial.Uniforms.at(i).Type;
				Meshes.at(j)->Uniforms.at(i).Content = (void*)Meshes.at(j)->MeshMaterial.Uniforms.at(i).Value.c_str();
			}
			Meshes.at(j)->ApplyUniforms();
		}
		catch (std::exception& e)
		{
			Log::Print(e.what());
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
	}
}

void Model::SimpleRender(Shader* UsedShader)
{
	if (!Visible) return;
	if (Size.isOnFrustum(FrustumCulling::CurrentCameraFrustum, ModelTransform.Location, ModelTransform.Scale * NonScaledSize * 0.025))
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
	size_t i = 0;
	for (auto& Uniform : Meshes[MeshIndex]->Uniforms)
	{
		if (Uniform.Name == NewUniform.UniformName)
		{
			Uniform.Type = NewUniform.Type;
			delete Uniform.Content;
			Uniform.Content = (void*)NewUniform.Value.c_str();
			Meshes[MeshIndex]->ApplyUniform(i);
			return;
		}
		i++;
	}
	Meshes[MeshIndex]->Uniforms.push_back(Uniform(NewUniform.UniformName, NewUniform.Type, (void*)NewUniform.Value.c_str()));
	Meshes[MeshIndex]->ApplyUniform(Meshes[MeshIndex]->Uniforms.size() - 1);
}