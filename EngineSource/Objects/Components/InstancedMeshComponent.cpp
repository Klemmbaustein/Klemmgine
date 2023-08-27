#include "InstancedMeshComponent.h"
#include <Rendering/Graphics.h>
#include <Engine/File/Assets.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Rendering/Mesh/InstancedModel.h>
#include <filesystem>
#include <Engine/Log.h>

InstancedMeshComponent::InstancedMeshComponent(std::string File)
{
	auto f = Assets::GetAsset(File + ".jsm");
	if (std::filesystem::exists(f))
	{
		Mesh = new InstancedModel(f);
		Graphics::MainFramebuffer->Renderables.push_back(Mesh);
	}
	else
	{
		Mesh = new InstancedModel("");
		Graphics::MainFramebuffer->Renderables.push_back(Mesh);
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
	if (!Mesh)
	{
		return;
	}
	for (auto* f : Graphics::AllFramebuffers)
	{
		for (size_t i = 0; i < f->Renderables.size(); i++)
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
	if (!Mesh)
	{
		return false;
	}
	return Mesh->AddInstance(T + GetParent()->GetTransform());
}

bool InstancedMeshComponent::RemoveInstance(size_t Index)
{
	if (!Mesh)
	{
		return false;
	}
	return Mesh->RemoveInstance(Index);
}

std::vector<size_t> InstancedMeshComponent::GetInstancesNearLocation(Vector3 Location, float Distance)
{
	if (!Mesh)
	{
		return {};
	}
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
	if (!Mesh)
	{
		return;
	}
	Mesh->ConfigureVAO();
}
