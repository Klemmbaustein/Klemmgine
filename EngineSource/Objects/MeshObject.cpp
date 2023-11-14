#include "MeshObject.h"
#include <Engine/Stats.h>
#include <filesystem>
#include <Engine/Log.h>
#include <Engine/File/Assets.h>

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
		AddEditorProperty(Property("Materials:Material " + std::to_string(i), Type::String, &MaterialNames[i]));
	}
	OnPropertySet();
}

void MeshObject::LoadFromFile(std::string Filename)
{
	if (Mesh) Detach(Mesh);
	for (CollisionComponent* Elem : MeshCollision)
	{
		Detach(Elem);
	}
	ModelGenerator::ModelData m;
	m.LoadModelFromFile(Filename);
	for (size_t i = 0; i < m.Elements.size(); i++)
	{
		if (i >= MaterialNames.size() || MaterialNames.at(i) == "")
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
#if !SERVER
	Mesh->GetModel()->CastShadow = Mesh->GetModel()->CastShadow && MeshCastShadow;
#endif
	Attach(Mesh);
	this->Filename = Filename;
	if (m.HasCollision || (IsInEditor && m.GetMergedVertices().size() < 5000))
	{
		ModelGenerator::ModelData CollDat;
		auto& CollElem = CollDat.AddElement();
		CollElem.Vertices = m.GetMergedVertices();
		CollElem.Indices = m.GetMergedIndices();

		if (m.GetMergedVertices().size() >= 5000)
		{
			CollDat.SeperateElementToGrid(0, std::max(200.0f, m.CollisionBox.GetLength() / 6));
		}
		for (auto& i : CollDat.Elements)
		{
			CollisionComponent* NewCollider = new CollisionComponent();
			NewCollider = new CollisionComponent();
			Attach(NewCollider);
			NewCollider->Init(i.Vertices, i.Indices, Transform(Vector3(), Vector3(), Vector3(1)));
			MeshCollision.push_back(NewCollider);
		}
	}

	Properties.clear();
	GenerateDefaultCategories();
	MaterialNames.clear();
	MaterialNames.resize(m.Elements.size());
	for (size_t i = 0; i < m.Elements.size(); i++)
	{
		MaterialNames[i] = m.Elements[i].ElemMaterial;
		if (MaterialNames[i].substr(0, 8) == "Content/")
		{
			MaterialNames[i] = MaterialNames[i].substr(8);
		}
		AddEditorProperty(Property("Materials:Material " + std::to_string(i), Type::String, &MaterialNames[i]));
	}
}

void MeshObject::OnPropertySet()
{
	if (!std::filesystem::exists(Assets::GetAsset(Filename + ".jsm")))
	{
		if (!Filename.empty())
		{
			Log::Print("Could find not model file: " + Filename + ".jsm", Log::LogColor::Yellow);
		}
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
#if !SERVER
	MaterialNames.clear();
	MaterialNames.resize(Mesh->GetModel()->ModelMeshData.Elements.size());
	for (size_t i = 0; i < Mesh->GetModel()->ModelMeshData.Elements.size(); i++)
	{
		MaterialNames[i] = Mesh->GetModel()->ModelMeshData.Elements[i].ElemMaterial;
		if (MaterialNames[i].substr(0, 8) == "Content/")
		{
			MaterialNames[i] = MaterialNames[i].substr(8);
		}
		AddEditorProperty(Property("Materials:Material " + std::to_string(i), Type::String, &MaterialNames[i]));
	}
#endif
}

void MeshObject::GenerateDefaultCategories()
{	
	// Categories are sorted alphabetically. The text renderer doesn't render newlines, so the categories have \n first so they will be sorted first.
	AddEditorProperty(Property("\nMesh:Mesh file", Type::String, &Filename));
	AddEditorProperty(Property("\nMesh:Cast Shadow", Type::Bool, &MeshCastShadow));
}
