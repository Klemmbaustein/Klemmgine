#pragma once
#include <Objects/Components/Component.h>

class InstancedModel;

/**
* @brief
* An instanced mesh component.
* 
* Renders multiple instances of the same mesh.
*/
class InstancedMeshComponent : public Component
{
public:
	InstancedMeshComponent(std::string File);
	void Begin() override;
	void Update() override;
	void Destroy() override;
	/**
	* @brief
	* Adds an instance at the given transform.
	* 
	* @param T
	* The transform of the instance
	* 
	* @return
	* The index of the instance.
	*/
	size_t AddInstance(Transform T);

	/**
	* @brief
	* Removes an instance from the mesh.
	* 
	* @param Index
	* The index of the instance.
	* 
	* @return
	* True if the instance was found from the index, false if not.
	*/
	bool RemoveInstance(size_t Index);
	
	/**
	* @brief
	* Gets all instances near the given position.
	* 
	* @param Position
	* The position where the check should happen.
	* 
	* @param Distance
	* If the distance of any instance to Position is less than distance, it will be returned.
	* 
	* @return
	* A list of all instances near the position.
	*/
	std::vector<size_t> GetInstancesNearPosition(Vector3 Position, float Distance);

	/// Updates all instances. This should be called each time an instance is added or removed.
	void UpdateInstances();
	InstancedModel* GetInstancedModel() { return Mesh; }
protected:
	InstancedModel* Mesh;
};