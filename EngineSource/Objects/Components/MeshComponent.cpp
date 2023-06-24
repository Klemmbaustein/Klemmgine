#include "MeshComponent.h"
#include <Rendering/Mesh/Model.h>
#include <World/Graphics.h>
#include <World/Assets.h>
#include <Rendering/Utility/Framebuffer.h>

void MeshComponent::Begin()
{
	if (MeshModel)
	{
		MeshModel->ModelTransform = GetParent()->GetTransform() + RelativeTransform;
	}
}

void MeshComponent::Destroy()
{
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
}
void MeshComponent::Tick()
{
	if (AutomaticallyUpdateTransform)
	{
		Vector3 InvertedRotation = (GetParent()->GetTransform().Rotation + RelativeTransform.Rotation);
		InvertedRotation = Vector3(-InvertedRotation.Z, InvertedRotation.Y, -InvertedRotation.X);
		MeshModel->ModelTransform = Transform(Vector3::TranslateVector(RelativeTransform.Location, GetParent()->GetTransform()),
			Vector3() - InvertedRotation.DegreesToRadiants(),
			RelativeTransform.Scale * GetParent()->GetTransform().Scale);
		MeshModel->UpdateTransform();
	}
}
void MeshComponent::Load(std::string File)
{
	if (MeshModel) delete MeshModel;
	MeshModel = new Model(Assets::GetAsset(File + ".jsm"));
	Graphics::MainFramebuffer->Renderables.push_back(MeshModel);
	MeshModel->UpdateTransform();
}

void MeshComponent::Load(ModelGenerator::ModelData Data)
{
	if (MeshModel) delete MeshModel;
	MeshModel = new Model(Data);
	Graphics::MainFramebuffer->Renderables.push_back(MeshModel);
	MeshModel->UpdateTransform();
}

FrustumCulling::AABB MeshComponent::GetBoundingBox()
{
	return MeshModel->Size;
}

void MeshComponent::SetUniform(std::string Name, Type::TypeEnum Type, std::string Content, uint8_t MeshSection)
{
	MeshModel->SetUniform(Material::Param(Name, Type, Content), MeshSection);
}


ModelGenerator::ModelData MeshComponent::GetModelData()
{
	return MeshModel->ModelMeshData;
}


void MeshComponent::SetVisibility(bool NewVisibility)
{
	MeshModel->Visible = NewVisibility;
}

void MeshComponent::UpdateTransform()
{
	if (!AutomaticallyUpdateTransform)
	{
		Vector3 InvertedRotation = (GetParent()->GetTransform().Rotation + RelativeTransform.Rotation);
		InvertedRotation = Vector3(InvertedRotation.Z, InvertedRotation.Y, InvertedRotation.X);
		MeshModel->ModelTransform = Transform(Vector3::TranslateVector(RelativeTransform.Location, GetParent()->GetTransform()),
			Vector3() - InvertedRotation,
			RelativeTransform.Scale * GetParent()->GetTransform().Scale);
		MeshModel->UpdateTransform();
	}
}
