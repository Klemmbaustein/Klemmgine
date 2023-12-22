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
class Component;

#define REGISTER_EVENT(InFunction, InType) do {\
NetEvent e;\
e.Name = # InFunction;\
e.Function = static_cast<NetEvent::NetEventFunction>(& InFunction);\
e.Parent = this;\
e.Type = InType;\
this->NetEvents.push_back(e);\
} while (0)


class WorldObject
{
public:
	struct NetEvent
	{
		std::string Name;
		typedef void(WorldObject::* NetEventFunction)(std::vector<std::string> Arguments);
		NetEventFunction Function;
		WorldObject* Parent;

		enum class EventType
		{
			// Event should be called on the server. Can only be called from the owning client.
			Server,
			// Event should be called on the owner. Can only be called from the server.
			Owner,
			// Event should be called on all clients. Can only be called from the server.
			Clients
		};

		EventType Type;
		
		void Invoke(std::vector<std::string> Arguments) const;
	};

	struct Property
	{
		Property(std::string Name, Type::TypeEnum Type, void* Data)
		{
			this->Name = Name;
			this->Type = Type;
			this->Data = Data;
		}
		Property(std::string Name, int Type, void* Data)
		{
			this->Name = Name;
			this->Type = (Type::TypeEnum)Type;
			this->Data = Data;
		}

		std::string Name;
		Type::TypeEnum Type;
		void* Data;
		enum class PropertyType
		{
			EditorProperty,
			NetProperty
		};
		enum class NetOwner
		{
			Client,
			Server
		};

		std::string ValueToString();

		NetOwner PropertyOwner = NetOwner::Server;

		PropertyType PType = PropertyType::EditorProperty;

	};

	void _CallEvent(NetEvent::NetEventFunction Function, std::vector<std::string> Arguments);

	template<typename T>
	void CallEvent(T Function, std::vector<std::string> Arguments = {})
	{
		_CallEvent(static_cast<NetEvent::NetEventFunction>(Function), Arguments);
	}


	WorldObject(ObjectDescription Descr = ObjectDescription("Empty Object", 0));
	virtual ~WorldObject();
	WorldObject* Start(std::string ObjectName, Transform Transform, uint64_t NetID);
	virtual void Destroy();
	virtual void Tick();
	virtual void Begin();
	virtual bool GetIsReplicated();

	void SetTransform(Transform NewTransform);
	Transform& GetTransform();
	int Attach(Component* NewComponent);
	void SetName(std::string Name);
	std::string GetName();
	ObjectDescription GetObjectDescription();
	void TickComponents();

	void AddEditorProperty(Property p);
	void AddNetProperty(Property p, Property::NetOwner Owner);

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
	void SetNetOwner(int64_t NewNetID);
	uint64_t NetID = 0;
	uint64_t NetOwner = UINT64_MAX;
	std::vector<Property> Properties;
	std::vector<NetEvent> NetEvents;
protected:
	std::string TypeName;
	uint32_t TypeID = 0;
	std::vector<Component*> Components;
	std::string Name = "Object";
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
	T* SpawnObject(Transform ObjectTransform, uint64_t NetID = UINT64_MAX);
	bool DestroyObject(WorldObject* Object);
	WorldObject* SpawnObjectFromID(uint32_t ID, Transform ObjectTransform, uint64_t NetID = UINT64_MAX);
	extern const std::vector<ObjectDescription> EditorObjects;
	std::vector<WorldObject*> GetAllObjectsWithID(uint32_t ID);
	extern std::vector<WorldObject*> AllObjects;
	std::string GetCategoryFromID(uint32_t ID);
}