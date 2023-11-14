#pragma once
#include "Objects/WorldObject.h"
#include <Objects/Components/CollisionComponent.h>
#include <Objects/Components/MeshComponent.h>
#include <GENERATED/MeshObject.h>

class Model;

class MeshObject : public WorldObject
{
public:
	MESHOBJECT_GENERATED("Default/Mesh")

	virtual void Destroy();
	virtual void Tick();
	virtual void Begin();
	void LoadFromFile(std::string Filename);
	virtual void OnPropertySet() override;
	bool MeshCastShadow = true;
protected:
	std::vector<CollisionComponent*> MeshCollision;
	void GenerateDefaultCategories();
	MeshComponent* Mesh = nullptr;
	std::string PreviousFilename;
	std::string Filename;
	std::vector<std::string> MaterialNames;
};