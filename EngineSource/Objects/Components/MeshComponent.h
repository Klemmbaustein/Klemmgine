#pragma once
#include <Objects/Components/Component.h>
#include <Rendering/Camera/FrustumCulling.h>
#include <Rendering/Mesh/ModelGenerator.h>

class Model;

class MeshComponent : public Component
{
public:
	virtual void Begin() override;
	virtual void Tick() override;
	virtual void Destroy() override;

	void Load(std::string File);
	void Load(ModelGenerator::ModelData Data);
	FrustumCulling::AABB GetBoundingBox();

	void SetUniform(std::string Name, Type::TypeEnum Type, std::string Content, uint8_t MeshSection);

	Model* GetModel()
	{
		return MeshModel;
	}
	ModelGenerator::ModelData GetModelData();
	void SetVisibility(bool NewVisibility);


	bool AutomaticallyUpdateTransform = true;
	void UpdateTransform();
protected:
	Model* MeshModel = nullptr;
};