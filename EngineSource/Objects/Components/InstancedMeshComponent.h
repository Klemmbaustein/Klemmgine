#pragma once
#include <Objects/Components/Component.h>

class InstancedModel;

class InstancedMeshComponent : public Component
{
public:
	InstancedMeshComponent(std::string File);
	void Begin() override;
	void Tick() override;
	void Destroy() override;
	size_t AddInstance(Transform T);
	bool RemoveInstance(size_t Index);
	std::vector<size_t> GetInstancesNearLocation(Vector3 Location, float Distance);
	void UpdateInstances();
	InstancedModel* GetInstancedModel() { return Mesh; }
protected:
	InstancedModel* Mesh;
};