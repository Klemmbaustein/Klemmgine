#pragma once
#include "Math/Vector.h"
#include <Engine/TypeEnun.h>
#include <set>

class EditorUI;
class MeshObject;

/**
* @brief
* A simple struct describing a WorldObject type.
*/
struct ObjectDescription
{
	ObjectDescription(std::string Name, uint32_t ID)
	{
		this->Name = Name;
		this->ID = ID;
	}
	/// The name of the object type.
	std::string Name;
	/// The ID of the object type.
	uint32_t ID;
};
class Component;

/**
* @def REGISTER_EVENT(InFunction, InType)
* 
* @brief
* Registers a WorldObject::NetEvent. Should only be used in a WorldObject.
*/
#define REGISTER_EVENT(InFunction, InType) do {\
NetEvent e;\
e.Name = # InFunction;\
e.Function = static_cast<NetEvent::NetEventFunction>(& InFunction);\
e.Parent = this;\
e.NativeType = InType;\
this->NetEvents.push_back(e);\
} while (0)

/**
* @defgroup Objects
* 
* @brief
* Functions/classes related to objects.
*/

/**
 * @brief
 * An object.
 * 
 * A class representing an object in a scene. It can have multiple components of the Component class.
 * 
 * C# equivalent: Engine.WorldObject.
 * 
 * Related: Objects namespace.
 * 
 * ## Reflection
 * 
 * A (very basic) type reflection system exists for WorldObjects in C++.
 * The "BuildTool" searches for header files (.h or .hpp) in the `Code/Objects` directory before each build.
 * It then generates GENERATED/ header files.
 * 
 * These header files include:
 * - A spawn list for Objects::SpawnObjectFromID().
 * - A 
 * 
 * ## Replication
 * 
 * If GetIsReplicated() is true, the object will be replicated between the client and server. The owner will be dictated by WorldObject::NetOwner.
 * 
 * @ingroup Objects
 */
class WorldObject
{
public:
	/**
	 * @brief
	 * A networked event that allows clients to call events on the server and the server to call events on clients.
	 * @ingroup Objects
	 */
	struct NetEvent
	{
		/**
		 * @brief
		 * Name of the event. Will be the name of the function if using the REGISTER_EVENT macro.
		 */
		std::string Name;
		typedef void(WorldObject::* NetEventFunction)(std::vector<std::string> Arguments);

		/**
		 * @brief
		 * Function pointer to the function corresponding to the event.
		 */
		NetEventFunction Function;

		/**
		 * @brief
		 * A pointer to the WorldObject owning this NetEvent.
		 */
		WorldObject* Parent;

		/**
		 * @brief
		 * Enum describing the type of the NetEvent.
		 */
		enum class EventType
		{
			/// Event should be called on the server. Can only be called from the owning client.
			Server,
			/// Event should be called on the owner. Can only be called from the server.
			Owner,
			/// Event should be called on all clients. Can only be called from the server.
			Clients
		};

		EventType NativeType;
		
		void Invoke(std::vector<std::string> Arguments) const;
	};

	struct Property
	{
		Property(std::string Name, NativeType::NativeType NativeType, void* Data)
		{
			this->Name = Name;
			this->NativeType = NativeType;
			this->Data = Data;
		}
		Property(std::string Name, int NativeType, void* Data)
		{
			this->Name = Name;
			this->NativeType = (NativeType::NativeType)NativeType;
			this->Data = Data;
		}
		Property()
		{

		}

		std::string Name;
		std::string ValueString;
		NativeType::NativeType NativeType = NativeType::Null;
		void* Data = nullptr;
		enum class PropertyType
		{
			EditorProperty,
			NetProperty,
#if ENGINE_CSHARP
			CSharpProperty
#endif
		};
		enum class NetOwner
		{
			Client,
			Server
		};

		std::string ValueToString(WorldObject* Context);

		NetOwner PropertyOwner = NetOwner::Server;

		PropertyType PType = PropertyType::EditorProperty;

	};

	void _CallEvent(NetEvent::NetEventFunction Function, std::vector<std::string> Arguments);

	/**
	 * @brief
	 * Calls the WorldObject::NetEvent that has the given function.
	 * 
	 * @code
	 * void MyObject::HelloWorld()
	 * {
	 *     Log::Print("Hello World");
	 * }
	 * 
	 * void MyObject::Begin()
	 * {
	 *     // Register an event that runs on the server.
	 *     REGISTER_EVENT(MyObject::HelloWorld, NetEvent::EventType::Server);
	 * }
	 * 
	 * void MyObject::Update()
	 * {
	 * #if !SERVER
	 *     if (Input::IsLMBClicked)
	 *     {
	 *         // Call the event registered in MyObject::Begin() on the client.
	 *         CallEvent(&MyObject::HelloWorld);
	 *         Log::Print("Called event on client");
	 *     }
	 * #endif
	 * }
	 * 
	 * // Output:
	 * // - Client: Called event on client
	 * // - Server: Hello World
	 * @endcode
	 * 
	 */
	template<typename T>
	void CallEvent(T Function, std::vector<std::string> Arguments = {})
	{
		_CallEvent(static_cast<NetEvent::NetEventFunction>(Function), Arguments);
	}


	WorldObject(ObjectDescription Descr = ObjectDescription("Empty Object", 0));
	virtual ~WorldObject();
	WorldObject* Start(std::string ObjectName, Transform Transform, uint64_t NetID);
	virtual void Destroy();
	virtual void Update();
	virtual void Begin();
	virtual bool GetIsReplicated();

	void SetTransform(Transform NewTransform);
	Transform& GetTransform();
	int Attach(Component* NewComponent);
	std::string Name = "Object";

	/**
	 * @brief
	 * Returns the ObjectDescription of the object.
	 */
	ObjectDescription GetObjectDescription();
	void UpdateComponents();

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

	/**
	 * @brief
	 * The scene name that this object belongs to. Related to subscenes.
	 * @deprecated
	 */
	std::string CurrentScene;
	static void DestroyMarkedObjects();
	
	/**
	 * @brief
	 * Sets the owner of an object to the given owner ID. Only works on the server.
	 * 
	 * Sets the NetOwner variable on the server and replicates the new value across all clients.
	 * On the client, this function has no effect.
	 * 
	 * @param NewNetID
	 * The new netID
	 */
	void SetNetOwner(int64_t NewNetID);

	/**
	 * @brief
	 * Unique identifier used to point to this object for networking.
	 */
	uint64_t NetID = 0;

	/**
	 * @brief
	 * The net owner of the WorldObject.
	 * 
	 * The default value is UINT64_MAX, or 0xffffffffffffffff, which is the ID for the server.
	 * This can be set across all clients using the SetNetOwner() function.
	 */
	uint64_t NetOwner = UINT64_MAX;
	std::vector<Property> Properties;
	std::vector<NetEvent> NetEvents;
protected:
	std::string TypeName;
	uint32_t TypeID = 0;
	std::vector<Component*> Components;
	friend EditorUI;
	friend class ContextMenu;
	Transform ObjectTransform;
private:
};

/**
 * @file
 */

/**
 * @brief
 * Namespace containing functions related to the WorldObject class.
 * @ingroup Objects
 */
namespace Objects
{
	extern std::set<WorldObject*> ObjectsToDestroy;

	/**
	 * @brief
	 * Creates and initializes a new WorldObject of type T.
	 * 
	 * @tparam T
	 * The type of the WorldObject.
	 * 
	 * @param ObjectTransform
	 * The Transform the object should start with.
	 * 
	 * @param NetID
	 * Internal. Used by the Networking system.
	 * 
	 * @ingroup Objects
	 */
	template<typename T>
	T* SpawnObject(Transform ObjectTransform, uint64_t NetID = UINT64_MAX);
	bool DestroyObject(WorldObject* Object);

	/**
	* @brief
	* Creates and initializes a new WorldObject with the type belonging to the typeID.
	* 
	* The ID of a WorldObject can be obtained with either:
	* `Object->GetObjectDescription().ID` or `Object::GetID()`
	* 
	* The SpawnObjectFromID() function uses a spawn list generated by the BuildTool.
	* This function calls Objects::SpawnObject().
	* 
	* @param ID
	* The ID of the object type.
	* 
	* @param ObjectTransform
	* The Transform the object should start with.
	*
	* @param NetID
	* Internal. Used by the Networking system.
	* 
	* @ingroup Objects
	*/
	WorldObject* SpawnObjectFromID(uint32_t ID, Transform ObjectTransform, uint64_t NetID = UINT64_MAX);
	/**
	* @brief
	* List of all WorldObject types.
	* 
	* Generated by the BuildTool.
	*/
	extern const std::vector<ObjectDescription> ObjectTypes;

	/**
	* @brief
	* Returns all objects that have a type with the matching ID.
	*/
	std::vector<WorldObject*> GetAllObjectsWithID(uint32_t ID);
	extern std::vector<WorldObject*> AllObjects;
	std::string GetCategoryFromID(uint32_t ID);
}