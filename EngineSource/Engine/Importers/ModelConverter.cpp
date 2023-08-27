#if EDITOR
#include "ModelConverter.h"
#include <filesystem>
#include <Engine/Log.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <vector>
#include <assimp/postprocess.h>
#include <UI/EditorUI/Popups/DialogBox.h>
#include <Engine/EngineError.h>

namespace fs = std::filesystem;
uint8_t NumMaterials = 0;

std::vector<ImportMesh> Meshes;
namespace Importer
{
	extern std::string From;
	extern std::string To;
}
void ProcessMesh(aiMesh* Mesh, const aiScene* Scene)
{
	ImportMesh m;
	for (uint32_t i = 0; i < Mesh->mNumVertices; i++)
	{
		m.Positions.push_back(Vector3(Mesh->mVertices[i].x, Mesh->mVertices[i].y, Mesh->mVertices[i].z));
		m.Normals.push_back(Vector3(Mesh->mNormals[i].x, Mesh->mNormals[i].y, Mesh->mNormals[i].z));
		if (Mesh->mTextureCoords[0])
			m.UVs.push_back(Vector2(Mesh->mTextureCoords[0][i].x, Mesh->mTextureCoords[0][i].y));
		else m.UVs.push_back(Vector2());
	}

	for (uint32_t i = 0; i < Mesh->mNumFaces; i++)
	{

		aiFace Face = Mesh->mFaces[i];
		ENGINE_ASSERT(Face.mNumIndices == 3, "Face should always have 3 indices.");
		for (unsigned int j = 0; j < Face.mNumIndices; j++)
		{
			m.Indicies.push_back(Face.mIndices[j]);
		}
	}
	Meshes.push_back(m);
}

void ProcessMaterials(const aiScene* Scene)
{
	NumMaterials = Scene->mNumMaterials;

}

void ProcessNode(aiNode* Node, const aiScene* Scene)
{
	for (uint32_t i = 0; i < Node->mNumMeshes; i++)
	{
		aiMesh* Mesh = Scene->mMeshes[Node->mMeshes[i]];
		ProcessMesh(Mesh, Scene);
	}

	for (uint32_t i = 0; i < Node->mNumChildren; i++)
	{
		ProcessNode(Node->mChildren[i], Scene);
	}
}

std::string ModelImporter::Import(std::string Name, std::string CurrentFilepath)
{
	if (fs::exists(Name))
	{
		Meshes.clear();
		Assimp::Importer Importer;
		const aiScene* Scene = Importer.ReadFile(Name, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_RemoveRedundantMaterials | aiProcess_PreTransformVertices);

		if (Scene == nullptr)
		{
			Log::Print(std::string("Error: Attempted to Import a non-3d Model as a 3d Model"), Vector3(1, 0.1f, 0.1f));
			return std::string("ERROR!");
		}

		if (!Scene || Scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode)
		{
			Log::Print("Error: Could not load Scene");
			if (Scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE)
				Log::Print(": AI_SCENE_FLAGS_INCOMPLETE");
			ENGINE_ASSERT(false, "Error: Could not load Scene");
		}
		ProcessMaterials(Scene);
		ProcessNode(Scene->mRootNode, Scene);

		std::string FileName = std::string(FileUtil::GetFileNameFromPath(Name));
		std::string FileNameWithoutExtension = FileName.substr(0, FileName.find_last_of("."));
		std::string OutputFileName = CurrentFilepath + "/" + FileNameWithoutExtension + ".jsm";
		if (std::filesystem::exists(OutputFileName))
		{
			Importer::From = Name;
			Importer::To = OutputFileName;
			new DialogBox("Model Import", 0, "\"" + FileUtil::GetFileNameWithoutExtensionFromPath(OutputFileName) + "\" already exists.",
			{
				DialogBox::Answer("Replace", []()
				{
					std::string TargetPath = Importer::To.substr(0, Importer::To.find_last_of("/\\"));
					std::filesystem::remove(Importer::To);
					Import(Importer::From, TargetPath);
				}), 
				DialogBox::Answer("Cancel", nullptr) 
			});
			return "";
		}
		std::ofstream Output(OutputFileName, std::ios::out | std::ios::binary);
		int iNumMaterials = NumMaterials;
		Output.write((char*)&iNumMaterials, sizeof(int));

		float Scale = FileUtil::GetExtension(Name) == "fbx" ? 1.0f : 100.0f;

		for (int j = 0; j < NumMaterials; j++)
		{
			ImportMesh CurrentMesh = Meshes.at(j);
			int NumVertices = (int)CurrentMesh.Positions.size();
			int NumIndices = (int)CurrentMesh.Indicies.size();

			Output.write((char*)&NumVertices, sizeof(int));
			Output.write((char*)&NumIndices, sizeof(int));

			for (auto& i : CurrentMesh.Positions)
			{
				i = i * Scale;
			}


			for (int i = 0; i < NumVertices; i++)
			{
				Output.write((char*)&CurrentMesh.Positions.at(i).X, sizeof(float));
				Output.write((char*)&CurrentMesh.Positions.at(i).Y, sizeof(float));
				Output.write((char*)&CurrentMesh.Positions.at(i).Z, sizeof(float));
				Output.write((char*)&CurrentMesh.Normals.at(i).X, sizeof(float));
				Output.write((char*)&CurrentMesh.Normals.at(i).Y, sizeof(float));
				Output.write((char*)&CurrentMesh.Normals.at(i).Z, sizeof(float));
				Output.write((char*)&CurrentMesh.UVs.at(i).X, sizeof(float));
				Output.write((char*)&CurrentMesh.UVs.at(i).Y, sizeof(float));
			}
			for (int i = 0; i < NumIndices; i++)
			{
				Output.write((char*)&CurrentMesh.Indicies.at(i), sizeof(int));
			}
			std::string DefaultMaterial = "NONE";
			size_t size = DefaultMaterial.size();
			Output.write((char*)&size, sizeof(size_t));
			Output.write(DefaultMaterial.c_str(), sizeof(char) * size);
		}
		bool CastShadow = true;
		bool TwoSided = false;
		Output.write((char*)&CastShadow, sizeof(uint8_t));
		Output.write((char*)&CastShadow, sizeof(uint8_t));
		Output.write((char*)&TwoSided, sizeof(uint8_t));
		Output.close();
		return OutputFileName;
	}
	else Log::Print(Name + " does not exist");
	return "";
}
#endif