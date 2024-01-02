#include <Engine/Scene.h>
#include <GENERATED/GENERATED_ObjectIncludes.h>
#include <Engine/Log.h>

// This file needs to be compiled per project since the objects are different for each project.

template<typename T>
T* Objects::SpawnObject(Transform ObjectTransform, uint64_t NetID)
{
	T* NewObject = new T();
	return dynamic_cast<T*>(NewObject->Start(NewObject->GetObjectDescription().Name, ObjectTransform, NetID));
}

bool Objects::DestroyObject(WorldObject* Object)
{
	if (Object)
	{
		Objects::ObjectsToDestroy.insert(Object);
		return true;
	}
	return false;
}

WorldObject* Objects::SpawnObjectFromID(uint32_t ID, Transform ObjectTransform, uint64_t NetID)
{
	switch (ID)
	{
#include <GENERATED/GENERATED_Spawnlist.h>
	default:
		Log::Print("Tried to spawn " + std::to_string(ID) + " but that ID does not exist!", Log::LogColor::Yellow);
		return nullptr;
	}
}

std::string Objects::GetCategoryFromID(uint32_t ID)
{
	switch (ID)
	{
#include <GENERATED/GENERATED_Categories.h>
	default:
		Log::Print("Tried to access category for object " + std::to_string(ID) + " but that ID does not exist!", Log::LogColor::Yellow);
		return "";
	}
}

namespace Objects
{
	const std::vector<ObjectDescription> ObjectTypes
	{
#include <GENERATED/GENERATED_ListOfObjects.h>
	};
}