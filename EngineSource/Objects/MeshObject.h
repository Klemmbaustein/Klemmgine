#pragma once
#include "Objects/WorldObject.h"
#include <Objects/Components/CollisionComponent.h>
#include <Objects/Components/MeshComponent.h>
#include <GENERATED/MeshObject.h>

class Model;

/**
* @brief
* An object representing a 3d mesh with collision in a scene.
* 
* @ingroup Objects
* 
* Path: Classes/Default/Mesh.
*/
class MeshObject : public WorldObject
{
public:
	MESHOBJECT_GENERATED("Default/Mesh")

	virtual void Destroy() override;
	virtual void Update() override;
	virtual void Begin() override;

	/**
	* @brief
	* Loads a mesh file (.jsm) from the given file name.
	* 
	* @param Filename
	* The name of the mesh file. (Without the path or extension)
	*/
	void LoadFromFile(std::string Filename);
	virtual void OnPropertySet() override;

	/**
	* @brief
	* True of the mesh should call a shadow. False if not.
	*/
	bool MeshCastShadow = true;
protected:
	std::vector<CollisionComponent*> MeshCollision;

	void GenerateDefaultCategories();
	MeshComponent* Mesh = nullptr;
	std::string PreviousFilename;
	std::string Filename;
	std::vector<std::string> MaterialNames;
};