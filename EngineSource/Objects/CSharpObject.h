#ifdef ENGINE_CSHARP
#pragma once
#include <GENERATED/CSharpObject.h>
#include <Objects/WorldObject.h>
#include <CSharp/CSharpInterop.h>

/**
* @brief
* A C++ object representing a managed C# object of the WorldObject class.
* 
* @ingroup Objects
* @ingroup CSharp
*/
class CSharpObject : public WorldObject
{
public:
	CSHARPOBJECT_GENERATED("Default");

	/// Struct containing information about the managed C# class this object manages.
	CSharp::CSharpWorldObject CS_Obj;

	void Begin() override;
	void Update() override;
	void Destroy() override;

	void Reload();

	Transform CSharpTransform;

	/// The class name of the object.
	std::string CSharpClass;
	std::string OldCSharpClass;

	void OnPropertySet() override;
	
	/**
	* @brief
	* Loads a class with the given class name.
	* 
	* @param ClassName
	* The name of the class that should be instantiated.
	*/
	void LoadClass(std::string ClassName);
};
#endif