#if !SERVER
#pragma once
#include <Rendering/Mesh/Model.h>
#include <Rendering/Mesh/InstancedMesh.h>
class InstancedModel : public Drawable
{
public:
	InstancedModel(std::string Filename);
	~InstancedModel();
	void Render(Camera* WorldCamera, bool MainFrameBuffer, bool TransparencyPass) override;
	virtual void SimpleRender(Shader* UsedShader) override;
	std::vector<glm::mat4> MatModel = { glm::mat4(1.f) };
	Collision::Box Size;
	Vector3 ModelCenter;
	Transform ModelTransform;
	bool TwoSided = false;
	size_t AddInstance(Transform T);
	bool RemoveInstance(size_t Inst);
	void ConfigureVAO();
	void LoadMaterials(std::vector<std::string> Materials);
	std::vector<Transform> Instances;
	std::vector<InstancedMesh*> Meshes;
protected:
	ModelGenerator::ModelData ModelData;
	unsigned int MatBuffer = -1;
	glm::mat4 ModelViewProjection = glm::mat4();
	Collision::Box NonScaledSize;
};
#endif