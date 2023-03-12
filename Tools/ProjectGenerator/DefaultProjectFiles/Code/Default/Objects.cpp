#include <Engine/Scene.h>
#include <GENERATED/GENERATED_ObjectIncludes.h>
#include <Engine/Log.h>

// This file needs to be compiled per project since the objects are different for each project.

template<typename T>
inline T* Objects::SpawnObject(Transform ObjectTransform)
{
	T* NewObject = new T();
	NewObject->Start(NewObject->GetObjectDescription().Name, ObjectTransform);
	NewObject->CurrentScene = Scene::CurrentScene;
	return NewObject;
}

// TODO: Objects::ObjectsToDestroy should be a set to avoid duplicates (duplicates are bad)

bool Objects::DestroyObject(WorldObject* Object)
{
	if (Object)
	{
		Objects::ObjectsToDestroy.insert(Object);
		return true;
	}
	return false;
}

WorldObject* Objects::SpawnObjectFromID(uint32_t ID, Transform ObjectTransform)
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
		Log::Print("Tried to access caregory for object " + std::to_string(ID) + " but that ID does not exist!", Log::LogColor::Yellow);
		return "";
	}
}

namespace Objects
{
	const std::vector<ObjectDescription> EditorObjects
	{
#include <GENERATED/GENERATED_ListOfObjects.h>
	};
}