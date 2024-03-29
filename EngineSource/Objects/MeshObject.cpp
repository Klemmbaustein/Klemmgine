#include "MeshObject.h"
#include <Engine/Stats.h>
#include <filesystem>
#include <Engine/Log.h>
#include <Engine/File/Assets.h>

void MeshObject::Destroy()
{
}

void MeshObject::Update()
{
	
}

void MeshObject::Begin()
{
	GenerateDefaultCategories();
	AddEditorProperty(Property("Materials:Materials", NativeType::String | NativeType::List, &MaterialNames));
	OnPropertySet();
}

void MeshObject::LoadFromFile(std::string Filename)
{
	if (Mesh) 
		Detach(Mesh);
	if (MeshCollision)
		Detach(MeshCollision);
	ModelGenerator::ModelData m;
	m.LoadModelFromFile(Filename);
	for (size_t i = 0; i < m.Elements.size(); i++)
	{
		if (i >= MaterialNames.size() || MaterialNames.at(i) == "")
		{
			continue;
		}
		m.Elements.at(i).ElemMaterial = MaterialNames.at(i);
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

		MeshCollision = new CollisionComponent();
		Attach(MeshCollision);
		MeshCollision->Load(m);
	}

#if !SERVER
	MaterialNames.resize(Mesh->GetModel()->ModelMeshData.Elements.size());
	for (size_t i = 0; i < Mesh->GetModel()->ModelMeshData.Elements.size(); i++)
	{
		MaterialNames[i] = Mesh->GetModel()->ModelMeshData.Elements[i].ElemMaterial;
	}
#endif
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
}

void MeshObject::GenerateDefaultCategories()
{	
	// Categories are sorted alphabetically. The text renderer doesn't render newlines, so the categories have \n first so they will be sorted first.
	AddEditorProperty(Property("\nMesh:Mesh file", NativeType::String, &Filename));
	AddEditorProperty(Property("\nMesh:Cast Shadow", NativeType::Bool, &MeshCastShadow));
}
