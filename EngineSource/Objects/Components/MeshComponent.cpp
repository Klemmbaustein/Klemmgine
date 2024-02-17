#include "MeshComponent.h"
#include <Rendering/Mesh/Model.h>
#include <Rendering/Graphics.h>
#include <Engine/File/Assets.h>
#include <Rendering/Utility/Framebuffer.h>

#if SERVER
static ModelGenerator::ModelData FallbackModelData;
#endif

void MeshComponent::Begin()
{
#if !SERVER
	if (MeshModel)
	{
		MeshModel->ModelTransform = GetParent()->GetTransform() + RelativeTransform;
	}
#endif
}

void MeshComponent::Destroy()
{
#if !SERVER
	for (auto* f : Graphics::AllFramebuffers)
	{
		for (int i = 0; i < f->Renderables.size(); i++)
		{
			if (MeshModel == f->Renderables[i])
			{
				f->Renderables.erase(f->Renderables.begin() + i);
			}
		}
	}
	delete MeshModel;
#endif
}
void MeshComponent::Update()
{
#if !SERVER
	if (AutomaticallyUpdateTransform)
	{
		MeshModel->ModelTransform = GetWorldTransform();
		MeshModel->UpdateTransform();
	}
#endif
}
void MeshComponent::Load(std::string File)
{
#if !SERVER
	if (MeshModel)
	{
		Destroy();
	}
	MeshModel = new Model(Assets::GetAsset(File + ".jsm"));
	Graphics::MainFramebuffer->Renderables.push_back(MeshModel);
	MeshModel->UpdateTransform();
#endif
}

void MeshComponent::Load(const ModelGenerator::ModelData& Data)
{
#if !SERVER
	if (MeshModel)
	{
		Destroy();
	}
	MeshModel = new Model(Data);
	Graphics::MainFramebuffer->Renderables.push_back(MeshModel);
	MeshModel->UpdateTransform();
#endif
}

FrustumCulling::AABB MeshComponent::GetBoundingBox()
{
#if !SERVER
	return MeshModel->Size;
#endif
	return FrustumCulling::AABB(glm::vec3(0), glm::vec3(0));
}

void MeshComponent::SetUniform(std::string Name, NativeType::NativeType NativeType, std::string Content, uint8_t MeshSection)
{
#if !SERVER
	MeshModel->SetUniform(Material::Param(Name, NativeType, Content), MeshSection);
#endif
}


const ModelGenerator::ModelData& MeshComponent::GetModelData()
{
#if !SERVER
	return MeshModel->ModelMeshData;
#else
	return FallbackModelData;
#endif
}


void MeshComponent::SetVisibility(bool NewVisibility)
{
#if !SERVER
	MeshModel->Visible = NewVisibility;
#endif
}

void MeshComponent::UpdateTransform()
{
#if !SERVER
	if (!AutomaticallyUpdateTransform)
	{
		Vector3 InvertedRotation = (GetParent()->GetTransform().Rotation + RelativeTransform.Rotation);
		InvertedRotation = Vector3(InvertedRotation.Z, InvertedRotation.Y, InvertedRotation.X);
		MeshModel->ModelTransform = Transform(Vector3::TranslateVector(RelativeTransform.Position, GetParent()->GetTransform()),
			Vector3() - InvertedRotation,
			RelativeTransform.Scale * GetParent()->GetTransform().Scale);
		MeshModel->UpdateTransform();
	}
#endif
}
