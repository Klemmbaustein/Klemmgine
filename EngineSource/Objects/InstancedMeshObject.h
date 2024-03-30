#pragma once
#include <Objects/Components/InstancedMeshComponent.h>
#include <GENERATED/InstancedMeshObject.h>

/**
* @brief
* An object representing multiple 3d meshes using the same mesh. Collision is not supported.
*
* @ingroup Objects
* Path: Classes/Default/Mesh.
*/
class InstancedMeshObject : public WorldObject
{
public:
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void Begin() override;
	void LoadFromFile(std::string Filename);
	virtual void OnPropertySet() override;
	size_t AddInstance(Transform T);
	INSTANCEDMESHOBJECT_GENERATED("Default/Mesh")

	virtual std::string Serialize() override;
	virtual void DeSerialize(std::string SerializedObject) override;
protected:
	bool Initialized = true, SoonInitialized = true;
	int Amount = 50;
	int Range = 1000;
	Vector3 Scale = Vector3(1);
	std::string Filename;
	std::string ComponentName;
	InstancedMeshComponent* IMComponent = nullptr;
};