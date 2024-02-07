#pragma once
#include <vector>
#include <string>
#include "Rendering/Camera/Camera.h"
#include "Math/Collision/CollisionBox.h"
#include <Rendering/Renderable.h>
#include <Rendering/Camera/FrustumCulling.h>
#include <Rendering/Mesh/ModelGenerator.h>
#include <Rendering/Texture/Material.h>

#if !SERVER
class Mesh;

class Model : public Renderable
{
public:
	Model(std::string Filename);

	Model(ModelGenerator::ModelData Data);

	Model()
	{
		CastShadow = true;
	}

	~Model();

	virtual void Render(Camera* WorldCamera, bool MainFrameBuffer, bool TransparencyPass) override;

	virtual void UpdateTransform()
	{
		MatModel = glm::mat4(1.f);

		Vector3 Scale = NonScaledSize * ModelTransform.Scale;
		
		ModelTransform.Scale = ModelTransform.Scale * 0.025f;
		MatModel = ModelTransform.ToMatrix();
		ModelTransform.Scale = ModelTransform.Scale / 0.025f;
		ConfigureVAO();
	}

	void MultiplyScale(Vector3 Multiplier)
	{
		ModelTransform.Scale.X *= Multiplier.X;
		ModelTransform.Scale.Y *= Multiplier.Y;
		ModelTransform.Scale.Z *= Multiplier.Z;
	}

	void Rotate(float Angle, Vector3 Axis)
	{
		ModelTransform.Rotation.X += Angle * Axis.X;
		ModelTransform.Rotation.Y += Angle * Axis.Y;
		ModelTransform.Rotation.Z += Angle * Axis.Z;
	}

	void AddOffset(Vector3 Offset)
	{
		ModelTransform.Position.X += Offset.X;
		ModelTransform.Position.Y += Offset.Y;
		ModelTransform.Position.Z += Offset.Z;
	}
	void ConfigureVAO();

	virtual void SimpleRender(Shader* UsedShader) override;
	glm::mat4 MatModel = glm::mat4(1.f);
	Vector3 ModelCenter;
	Transform ModelTransform;
	bool TwoSided = false, HasCollision = true;
	FrustumCulling::AABB Size;
	void SetUniform(Material::Param NewUniform, uint8_t MeshIndex);

	bool ShouldCull = false;
	bool RunningQuery = false;
	uint8_t OcclusionQueryIndex = 255;
	bool IsOcclusionCulled = false;
	bool Visible = true;

	unsigned int MatBuffer = -1;
	std::vector<Mesh*> Meshes;
	ModelGenerator::ModelData ModelMeshData;
protected:
	glm::mat4 ModelViewProjection = glm::mat4(1);
	float NonScaledSize = 1;
};
#endif