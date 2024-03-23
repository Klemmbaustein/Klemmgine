#ifdef ENGINE_CSHARP
#pragma once
#include <GENERATED/CSharpObject.h>
#include <Objects/WorldObject.h>
#include <Engine/Subsystem/CSharpInterop.h>

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
	CSharpInterop::CSharpWorldObject CS_Obj;

	void Begin() override;
	void Update() override;
	void Destroy() override;

	void Reload(bool DeleteParameters);

	std::string GetProperty(std::string PropertyName) const;
	void SetProperty(std::string PropertyName, std::string Value);


	/// The class name of the object.
	std::string CSharpClass;


	void OnPropertySet() override;
	
	/**
	* @brief
	* Loads a class with the given class name.
	* 
	* @param ClassName
	* The name of the class that should be instantiated.
	*/
	void LoadClass(std::string ClassName);

private:
	Transform CSharpTransform;
	std::string OldCSharpClass;
};
#endif