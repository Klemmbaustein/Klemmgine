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

			Collision::HitResponse Hit = Collision::LineTrace(GetTransform().Location + Loc, GetTransform().Location + Loc - Vector3(0, 100, 0));
			Vector3 AxisA = Vector3(Hit.Normal.Y, Hit.Normal.Z, Hit.Normal.X);
			Vector3 AxisB = Vector3::Cross(Hit.Normal, AxisA);
			Vector3 Rotation = Vector3(sin(AxisB.Y), atan2(AxisB.X, AxisB.Z) + 3.14f, 0).RadiansToDegrees();
			Rotation = Vector3(Rotation.X, 0, Rotation.Y).DegreesToRadians();
			if (Hit.Hit)
			{
				if (ComponentName == "")
				{
					AddInstance(Transform(Hit.ImpactPoint - GetTransform().Location,
						Rotation,
						Scale * Random::GetRandomFloat(15, 25) / 20));
				}
				else if (ComponentName == Hit.HitObject->Name)
				{
					AddInstance(Transform(Hit.ImpactPoint - GetTransform().Location,
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
	AddEditorProperty(Property("Mesh", Type::String, &Filename));
	AddEditorProperty(Property("NumInstances", Type::Int, &Amount));
	AddEditorProperty(Property("Range", Type::Int, &Range));
	AddEditorProperty(Property("Component Name", Type::String, &ComponentName));
	AddEditorProperty(Property("Scale", Type::Vector3, &Scale));

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

void InstancedMeshObject::Deserialize(std::string SerializedObject)
{
}
