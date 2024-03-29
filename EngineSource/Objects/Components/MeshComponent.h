#pragma once
#include <Objects/Components/Component.h>
#include <Rendering/Camera/FrustumCulling.h>
#include <Rendering/Mesh/ModelGenerator.h>

class Model;

/**
* @brief
* A component containing mesh information.
* 
* @ingroup Components
*/
class MeshComponent : public Component
{
public:
	virtual void Begin() override;
	virtual void Update() override;
	virtual void Destroy() override;

	/**
	* @brief
	* Loads a mesh file (.jsm) from the given file name. (without path or extension)
	*
	* @param File
	* The file to load.
	*/
	void Load(std::string File);

	/**
	* @brief
	* Loads a mesh from the given model data struct.
	*
	* @param Data
	* The model data struct.
	*/
	void Load(const ModelGenerator::ModelData& Data);
	FrustumCulling::AABB GetBoundingBox();

	/**
	* @brief
	* Sets a material uniform with the given name and type the value of `Content`
	* 
	* 
	* @param Name
	* The name of the uniform.
	* 
	* @param NativeType
	* The type of the uniform.
	* Possible values:
	* 
	* @param Content
	* the new value of the uniform.
	* 
	* @param MeshSection
	* The section of the mesh that should have the uniform set.
	*/
	void SetUniform(std::string Name, NativeType::NativeType NativeType, std::string Content, uint8_t MeshSection);

	Model* GetModel()
	{
		return MeshModel;
	}
	const ModelGenerator::ModelData& GetModelData();
	void SetVisibility(bool NewVisibility);

	bool CastStaticShadow = true;
	bool AutomaticallyUpdateTransform = true;
	void UpdateTransform();
protected:
	Model* MeshModel = nullptr;
};