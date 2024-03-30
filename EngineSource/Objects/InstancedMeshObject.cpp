#include "InstancedMeshObject.h"
#include <Engine/EngineRandom.h>
#include <Math/Collision/Collision.h>
void InstancedMeshObject::Destroy()
{
}


void InstancedMeshObject::Update()
{
	if (!Initialized)
	{
		for (int i = 0; i < Amount; i++)
		{
			Vector3 Loc = Vector3(Random::GetRandomFloat(-(float)Range, (float)Range),
				0, Random::GetRandomFloat(-(float)Range, (float)Range));

			Collision::HitResponse Hit = Collision::LineTrace(GetTransform().Position + Loc, GetTransform().Position + Loc - Vector3(0, 100, 0));


			Vector3 Rotation = Vector3::LookAtFunctionY(0, Hit.Normal);
			if (Hit.Hit)
			{
				if (ComponentName == "")
				{
					AddInstance(Transform(Hit.ImpactPoint - GetTransform().Position,
						Rotation,
						Scale * Random::GetRandomFloat(15, 25) / 20));
				}
				else if (ComponentName == Hit.HitObject->Name)
				{
					AddInstance(Transform(Hit.ImpactPoint - GetTransform().Position,
						Rotation,
						Scale * Random::GetRandomFloat(15, 25) / 20));
				}
			}

		}
		IMComponent->UpdateInstances();
		Initialized = true;
	}
	if (!SoonInitialized)
	{
		Initialized = false;
		SoonInitialized = true;
	}
}

void InstancedMeshObject::Begin()
{
	AddEditorProperty(Property("Mesh", NativeType::String, &Filename));
	AddEditorProperty(Property("NumInstances", NativeType::Int, &Amount));
	AddEditorProperty(Property("Range", NativeType::Int, &Range));
	AddEditorProperty(Property("Component Name", NativeType::String, &ComponentName));
	AddEditorProperty(Property("Scale", NativeType::Vector3, &Scale));

	IMComponent = nullptr;

}

void InstancedMeshObject::LoadFromFile(std::string Filename)
{
}

void InstancedMeshObject::OnPropertySet()
{
	if (IMComponent)
	{
		Detach(IMComponent);
	}
	IMComponent = new InstancedMeshComponent(Filename);
	Attach(IMComponent);
	SoonInitialized = false;
}

size_t InstancedMeshObject::AddInstance(Transform T)
{
	return IMComponent->AddInstance(T);
}

std::string InstancedMeshObject::Serialize()
{
	return std::string();
}

void InstancedMeshObject::DeSerialize(std::string SerializedObject)
{
}
