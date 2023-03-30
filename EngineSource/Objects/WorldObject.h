#pragma once
#include "Math/Vector.h"
#include <Engine/TypeEnun.h>
#include <set>

class EditorUI;
class MeshObject;

struct ObjectDescription
{
	ObjectDescription(std::string Name, uint32_t ID)
	{
		this->Name = Name;
		this->ID = ID;
	}
	std::string Name;
	uint32_t ID;
};

namespace Objects
{
	struct Property
	{
		Property(std::string Name, Type::TypeEnum Type, void* Data)
		{
			this->Name = Name;
			this->Type = Type;
			this->Data = Data;
		}
		std::string Name;
		Type::TypeEnum Type;
		void* Data;
		bool operator==(Property b) const
		{
			return Name == b.Name && Type == b.Type && Data == b.Data;
		}
	};
}

class Component;

class WorldObject
{
public:
	WorldObject(ObjectDescription _d = ObjectDescription("Empty Object", 0));
	virtual ~WorldObject();
	void Start(std::string ObjectName, Transform Transform);
	virtual void Destroy();
	virtual void Tick();
	virtual void Begin();
	void SetTransform(Transform NewTransform);
	Transform& GetTransform();
	int Attach(Component* NewComponent);
	void SetName(std::string Name);
	std::string GetName();
	ObjectDescription GetObjectDescription();
	void TickComponents();
	std::vector<Component*> GetComponents()
	{
		return Components;
	}
	virtual std::string Serialize();
	void Detach(Component* C);
	virtual void Deserialize(std::string SerializedObject);
	virtual void OnPropertySet();
	std::string GetPropertiesAsString();
	void LoadProperties(std::string in);
	bool IsSelected = false;
	std::string CurrentScene;
	static void DestroyMarkedObjects();
protected:
	std::vector<Objects::Property> Properties;
	std::string TypeName;
	uint32_t TypeID = 0;
	std::vector<Component*> Components;
	std::string Name = "Empty Object";
	friend EditorUI;
	friend class ContextMenu;
	Transform ObjectTransform;
private:
};

namespace Objects
{
	extern std::set<WorldObject*> ObjectsToDestroy;
	//Creating or destroying objects is expensive and should be avoided if possible
	template<typename T>
	T* SpawnObject(Transform ObjectTransform);
	bool DestroyObject(WorldObject* Object);
	WorldObject* SpawnObjectFromID(uint32_t ID, Transform ObjectTransform);
	extern const std::vector<ObjectDescription> EditorObjects;
	std::vector<WorldObject*> GetAllObjectsWithID(uint32_t ID);
	extern std::vector<WorldObject*> AllObjects;
	std::string GetCategoryFromID(uint32_t ID);
}