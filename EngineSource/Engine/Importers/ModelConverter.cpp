#if EDITOR
#include "ModelConverter.h"
#include <filesystem>
#include <Engine/Log.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <vector>
#include <assimp/postprocess.h>
#include <UI/EditorUI/EditorUI.h>

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
		assert(Face.mNumIndices == 3);
		for (int j = 0; j < Face.mNumIndices; j++)
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
				std::cout << ": AI_SCENE_FLAGS_INCOMPLETE";
			throw "model loading error!";
			return "";
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
			//Editor::CurrentUI->ShowPopUpWindow("File already exists!", { PopUpButton("Replace", true, []()
			//	{
			//		std::string TargetPath = Importer::To.substr(0, Importer::To.find_last_of("/\\"));
			//		std::filesystem::remove(Importer::To);
			//		Log::Print(TargetPath, Log::LogColor::Blue);
			//		Import(Importer::From, TargetPath);
			//	}
			//), PopUpButton("Cancel", false, nullptr) });
			return "";
		}
		std::ofstream Output(OutputFileName, std::ios::out | std::ios::binary);
		int iNumMaterials = NumMaterials;
		Output.write((char*)&iNumMaterials, sizeof(int));

		for (int j = 0; j < NumMaterials; j++)
		{
			ImportMesh CurrentMesh = Meshes.at(j);
			int NumVertices = CurrentMesh.Positions.size();
			int NumIndices = CurrentMesh.Indicies.size();

			Output.write((char*)&NumVertices, sizeof(int));
			Output.write((char*)&NumIndices, sizeof(int));


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
			std::string DefaultMaterial = "Content/NONE";
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