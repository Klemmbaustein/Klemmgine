#include "MeshObject.h"
#include <World/Stats.h>
#include <filesystem>
#include <Engine/Log.h>
#include <World/Assets.h>

void MeshObject::Destroy()
{
}

void MeshObject::Tick()
{
	
}

void MeshObject::Begin()
{
	GenerateDefaultCategories();
	MaterialNames.resize(16);
	for (size_t i = 0; i < 16; i++)
	{
		MaterialNames[i] = "";
		Properties.push_back(Objects::Property("Materials:Material " + std::to_string(i), Type::E_STRING, &MaterialNames[i]));
	}
	OnPropertySet();
}

void MeshObject::LoadFromFile(std::string Filename)
{
	if (Mesh) Detach(Mesh);
	if (MeshCollision) Detach(MeshCollision);
	ModelGenerator::ModelData m;
	m.LoadModelFromFile(Filename);
	for (size_t i = 0; i < m.Elements.size(); i++)
	{
		if (i >= MaterialNames.size() || MaterialNames[i] == "")
		{
			continue;
		}
		if (MaterialNames[i].substr(0, 8) != "Content/")
		{
			m.Elements.at(i).ElemMaterial = "Content/" + MaterialNames.at(i);
		}
		else
		{
			m.Elements.at(i).ElemMaterial = MaterialNames.at(i);
		}
	}

	Mesh = new MeshComponent();
	Mesh->Load(m);
	Mesh->GetModel()->CastShadow = Mesh->GetModel()->CastShadow && MeshCastShadow;
	Attach(Mesh);
	this->Filename = Filename;
	if (Mesh->GetModel()->HasCollision || (IsInEditor && Mesh->GetModelData().GetMergedVertices().size() < 5000))
	{
		MeshCollision = new CollisionComponent();
		Attach(MeshCollision);
		MeshCollision->Init(Mesh->GetModelData().GetMergedVertices(), Mesh->GetModelData().GetMergedIndices(), Transform(Vector3(), Vector3(), Vector3(1)));
	}

	Properties.clear();
	GenerateDefaultCategories();
	MaterialNames.clear();
	MaterialNames.resize(Mesh->GetModel()->ModelMeshData.Elements.size());
	for (size_t i = 0; i < Mesh->GetModel()->ModelMeshData.Elements.size(); i++)
	{
		MaterialNames[i] = Mesh->GetModel()->ModelMeshData.Elements[i].ElemMaterial;
		if (MaterialNames[i].substr(0, 8) == "Content/")
		{
			MaterialNames[i] = MaterialNames[i].substr(8);
		}
		Properties.push_back(Objects::Property("Materials:Material " + std::to_string(i), Type::E_STRING, &MaterialNames[i]));
	}
}

void MeshObject::OnPropertySet()
{
	if (!std::filesystem::exists(Assets::GetAsset(Filename + ".jsm")))
	{
		return;
	}
	if (Filename != PreviousFilename)
	{
		if (!PreviousFilename.empty())
		{
			MaterialNames.clear();
		}
		PreviousFilename = Filename;
	}
	LoadFromFile(Filename);
	Properties.clear();
	GenerateDefaultCategories();
	MaterialNames.clear();
	MaterialNames.resize(Mesh->GetModel()->ModelMeshData.Elements.size());
	for (size_t i = 0; i < Mesh->GetModel()->ModelMeshData.Elements.size(); i++)
	{
		MaterialNames[i] = Mesh->GetModel()->ModelMeshData.Elements[i].ElemMaterial;
		if (MaterialNames[i].substr(0, 8) == "Content/")
		{
			MaterialNames[i] = MaterialNames[i].substr(8);
		}
		Properties.push_back(Objects::Property("Materials:Material " + std::to_string(i), Type::E_STRING, &MaterialNames[i]));
	}
}

void MeshObject::GenerateDefaultCategories()
{	
	// Categories are sorted alphabetically. The text renderer doesn't render newlines, so the categories have \n first so they will be sorted first.
	Properties.push_back(Objects::Property("\nMesh:Mesh file", Type::E_STRING, &Filename));
	Properties.push_back(Objects::Property("\nMesh:Cast Shadow", Type::E_BOOL, &MeshCastShadow));
}
