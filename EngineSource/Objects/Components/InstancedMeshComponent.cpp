#include "InstancedMeshComponent.h"
#include <World/Graphics.h>
#include <World/Assets.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Rendering/Mesh/InstancedModel.h>

InstancedMeshComponent::InstancedMeshComponent(std::string File)
{
	if (!File.empty())
	{
		Mesh = new InstancedModel(Assets::GetAsset(File + ".jsm"));
		Graphics::MainFramebuffer->Renderables.push_back(Mesh);
	}
	else
	{
		Mesh = nullptr;
	}
}

void InstancedMeshComponent::Begin()
{
}

void InstancedMeshComponent::Tick()
{
}

void InstancedMeshComponent::Destroy()
{
	for (auto* f : Graphics::AllFramebuffers)
	{
		for (int i = 0; i < f->Renderables.size(); i++)
		{
			if (Mesh == f->Renderables[i])
			{
				f->Renderables.erase(f->Renderables.begin() + i);
			}
		}
	}
	delete Mesh;
}

size_t InstancedMeshComponent::AddInstance(Transform T)
{
	return Mesh->AddInstance(T + GetParent()->GetTransform());
}

bool InstancedMeshComponent::RemoveInstance(size_t Index)
{
	return Mesh->RemoveInstance(Index);
}

std::vector<size_t> InstancedMeshComponent::GetInstancesNearLocation(Vector3 Location, float Distance)
{
	std::vector<size_t> Instances;
	for (size_t i = 0; i < Mesh->Instances.size(); i++)
	{
		if (Vector3::Distance(Mesh->Instances[i].Location, Location) <= Distance)
		{
			Instances.push_back(i);
		}
	}
	return Instances;
}

void InstancedMeshComponent::UpdateInstances()
{
	Mesh->ConfigureVAO();
}
