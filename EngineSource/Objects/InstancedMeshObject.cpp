#include "InstancedMeshObject.h"
#include <Engine/EngineRandom.h>
#include <Math/Collision/Collision.h>
void InstancedMeshObject::Destroy()
{
}


void InstancedMeshObject::Tick()
{
	if (!Initialized)
	{
		for (int i = 0; i < Amount; i++)
		{
			Vector3 Loc = Vector3(Random::GetRandomFloat(-Range, Range),
				0, Random::GetRandomFloat(-Range, Range));

			Collision::HitResponse Hit = Collision::LineTrace(GetTransform().Location + Loc, GetTransform().Location + Loc - Vector3(0, 100, 0));
			Vector3 AxisA = Vector3(Hit.Normal.Y, Hit.Normal.Z, Hit.Normal.X);
			Vector3 AxisB = Vector3::Cross(Hit.Normal, AxisA);
			Vector3 Rotation = Vector3(sin(AxisB.Y), atan2(AxisB.X, AxisB.Z) + 3.14, 0).RadiantsToDegrees();
			Rotation = Vector3(Rotation.X, 0, Rotation.Y).DegreesToRadiants();
			if (Hit.Hit)
			{
				if (ComponentName == "")
				{
					AddInstance(Transform(Hit.ImpactPoint - GetTransform().Location,
						Rotation,
						Scale * Random::GetRandomFloat(15, 25) / 20));
				}
				else if (ComponentName == Hit.HitObject->GetName())
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
	Properties.push_back(Objects::Property("Mesh", Type::E_STRING, &Filename));
	Properties.push_back(Objects::Property("NumInstances", Type::E_INT, &Amount));
	Properties.push_back(Objects::Property("Range", Type::E_INT, &Range));
	Properties.push_back(Objects::Property("Component Name", Type::E_STRING, &ComponentName));
	Properties.push_back(Objects::Property("Scale", Type::E_VECTOR3, &Scale));

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
