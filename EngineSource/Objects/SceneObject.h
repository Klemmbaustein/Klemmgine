#pragma once
#include "Math/Vector.h"
#include <Engine/TypeEnun.h>
#include <set>

class EditorUI;
class MeshObject;

/**
* @brief
* A simple struct describing a SceneObject type.
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
* Registers a SceneObject::NetEvent. Should only be used in a SceneObject.
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
 * C# equivalent: Engine.SceneObject.
 * 
 * Related: Objects namespace.
 * 
 * ## Reflection
 * 
 * A (very basic) type reflection system exists for SceneObjects in C++.
 * The "BuildTool" searches for header files (.h or .hpp) in the `Code/Objects` directory before each build.
 * It then generates GENERATED/ header files.
 * 
 * These header files include:
 * - A spawn list for Objects::SpawnObjectFromID().
 * - A 
 * 
 * ## Replication
 * 
 * If GetIsReplicated() is true, the object will be replicated between the client and server. The owner will be dictated by SceneObject::NetOwner.
 * 
 * @ingroup Objects
 */
class SceneObject
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
		typedef void(SceneObject::* NetEventFunction)(std::vector<std::string> Arguments);

		/**
		 * @brief
		 * Function pointer to the function corresponding to the event.
		 */
		NetEventFunction Function = nullptr;

		/**
		 * @brief
		 * A pointer to the SceneObject owning this NetEvent.
		 */
		SceneObject* Parent = nullptr;

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

		EventType NativeType = EventType::Server;
		
		void Invoke(std::vector<std::string> Arguments) const;
	};

	/// A property of the object.
	struct Property
	{
		/// Constructs a property from the name, type and pointer to the property data.
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

		/// The name of the property.
		std::string Name;
		std::string ValueString;
		/// The type of the property data.
		NativeType::NativeType NativeType = NativeType::Null;
		/// The property's data pointer.
		void* Data = nullptr;
		enum class PropertyType
		{
			/// The property is saved in the scene file and can be viewed and modified in the editor UI.
			EditorProperty,
			/// The property is replicated between the server and clients.
			NetProperty,
#if ENGINE_CSHARP
			/// Like EditorProperty, but the property is defined in C# code.
			CSharpProperty
#endif
		};

		/// The owner of a net property.
		enum class NetOwner
		{
			/// The property is owned by the client and is replicated to the server.
			Client,
			/// The property is owned by the server and is replicated to the client.
			Server
		};

		/// Converts the value at Data to a string.
		std::string ValueToString(SceneObject* Context);

		NetOwner PropertyOwner = NetOwner::Server;

		/// The type of the property.
		PropertyType PType = PropertyType::EditorProperty;

	};

	/**
	 * @brief
	 * Calls the SceneObject::NetEvent that has the given function.
	 * 
	 * @code
	 * void MyObject::HelloWorld(std::vector<std::string>)
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
		CallEventInternal(static_cast<NetEvent::NetEventFunction>(Function), Arguments);
	}


	SceneObject(ObjectDescription Descr = ObjectDescription("Empty Object", 0));
	virtual ~SceneObject();
	SceneObject* Start(std::string ObjectName, Transform Transform, uint64_t NetID);
	virtual void Destroy();
	virtual void Update();
	virtual void Begin();
	virtual bool GetIsReplicated();

	/// Sets the transform of the object.
	void SetTransform(Transform NewTransform);
	/// Gets the transform of the object.
	Transform& GetTransform();
	/// Attaches a Component to the object.
	int Attach(Component* NewComponent);
	/// The name of the object.
	std::string Name = "Object";

	/**
	 * @brief
	 * Returns the ObjectDescription of the object.
	 */
	ObjectDescription GetObjectDescription();
	void UpdateComponents();

	/// Adds an editor property to the object. Editor properties will be saved in the scene file, and can be viewed and modified in the editor UI.
	void AddEditorProperty(Property p);
	/// Adds a net property to the object. Editor properties are replicated between the server and clients.
	void AddNetProperty(Property p, Property::NetOwner Owner);

	/// Gets all components of the object.
	std::vector<Component*> GetComponents()
	{
		return Components;
	}
	virtual std::string Serialize();
	/// Detaches and destroys a component from this object.
	void Detach(Component* C);
	virtual void DeSerialize(std::string SerializedObject);

	/// This function is called when an editor property (Added with AddEditorProperty()) is set.
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
	static void DestroyMarkedObjects(bool SendNetworkEvents);
	
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
	 * The net owner of the SceneObject.
	 * 
	 * The default value is UINT64_MAX, or 0xffffffffffffffff, which is the ID for the server.
	 * This can be set across all clients using the SetNetOwner() function.
	 */
	uint64_t NetOwner = UINT64_MAX;
	std::vector<Property> Properties;
	std::vector<NetEvent> NetEvents;
protected:
	void CallEventInternal(NetEvent::NetEventFunction Function, std::vector<std::string> Arguments);
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
 * Namespace containing functions related to the SceneObject class.
 * @ingroup Objects
 */
namespace Objects
{
	extern std::set<SceneObject*> ObjectsToDestroy;

	/**
	 * @brief
	 * Creates and initializes a new SceneObject of type T.
	 * 
	 * @tparam T
	 * The type of the SceneObject.
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
	bool DestroyObject(SceneObject* Object);

	/**
	* @brief
	* Creates and initializes a new SceneObject with the type belonging to the typeID.
	* 
	* The ID of a SceneObject can be obtained with either:
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
	SceneObject* SpawnObjectFromID(uint32_t ID, Transform ObjectTransform, uint64_t NetID = UINT64_MAX);
	/**
	* @brief
	* List of all SceneObject types.
	* 
	* Generated by the BuildTool.
	*/
	extern const std::vector<ObjectDescription> ObjectTypes;

	/**
	* @brief
	* Returns all objects that have a type with the matching ID.
	*/
	std::vector<SceneObject*> GetAllObjectsWithID(uint32_t ID);
	extern std::vector<SceneObject*> AllObjects;
	std::string GetCategoryFromID(uint32_t ID);
}