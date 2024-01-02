#include "InstancedMeshComponent.h"
#include <Rendering/Graphics.h>
#include <Engine/File/Assets.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Rendering/Mesh/InstancedModel.h>
#include <filesystem>
#include <Engine/Log.h>

InstancedMeshComponent::InstancedMeshComponent(std::string File)
{
#if !SERVER
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
#endif
}

void InstancedMeshComponent::Begin()
{
}

void InstancedMeshComponent::Update()
{
}

void InstancedMeshComponent::Destroy()
{
#if !SERVER
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
#endif
}

size_t InstancedMeshComponent::AddInstance(Transform T)
{
#if !SERVER
	if (!Mesh)
	{
		return false;
	}
	return Mesh->AddInstance(T + GetParent()->GetTransform());
#endif
	return 0;
}

bool InstancedMeshComponent::RemoveInstance(size_t Index)
{
#if !SERVER
	if (!Mesh)
	{
		return false;
	}
	return Mesh->RemoveInstance(Index);
#endif
	return false;
}

std::vector<size_t> InstancedMeshComponent::GetInstancesNearLocation(Vector3 Location, float Distance)
{
	if (!Mesh)
	{
		return {};
	}
	std::vector<size_t> Instances;
#if !SERVER
	for (size_t i = 0; i < Mesh->Instances.size(); i++)
	{
		if (Vector3::Distance(Mesh->Instances[i].Location, Location) <= Distance)
		{
			Instances.push_back(i);
		}
	}
#endif
	return Instances;
}

void InstancedMeshComponent::UpdateInstances()
{
#if !SERVER
	if (!Mesh)
	{
		return;
	}
	Mesh->ConfigureVAO();
#endif
}
