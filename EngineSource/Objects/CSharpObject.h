#ifdef ENGINE_CSHARP
#pragma once
#include <GENERATED/GENERATED_CSharpObject.h>
#include <Objects/WorldObject.h>
#include <CSharp/CSharpInterop.h>

class CSharpObject : public WorldObject
{
public:
	CSHARPOBJECT_GENERATED("Default");

	CSharp::CSharpWorldObject CS_Obj;

	void Begin() override;
	void Tick() override;
	void Destroy() override;

	void Reload();

	Transform CSharpTransform;

	std::string CSharpClass;
	std::string OldCSharpClass;

	void OnPropertySet() override;
	
	void LoadClass(std::string ClassName);
};
#endif