#ifdef ENGINE_CSHARP
#include "CSharpObject.h"
#include <Engine/Log.h>

void CSharpObject::Begin()
{
	AddEditorProperty(Property("C#:Object class", Type::String, &CSharpClass));
}

void CSharpObject::Tick()
{
	if (!CS_Obj.ID)
	{
		return;
	}

	CSharp::ExectuteFunctionOnObject(CS_Obj, "TickComponents");
#if !EDITOR
	CSharp::ExectuteFunctionOnObject(CS_Obj, "Tick");
#endif
}

void CSharpObject::Destroy()
{
	CSharp::DestroyObject(CS_Obj);
}

void CSharpObject::Reload()
{
	OldCSharpClass = CSharpClass;
	if (CS_Obj.ID)
	{
		CSharp::DestroyObject(CS_Obj);
	}
	CS_Obj = CSharp::InstantiateObject(CSharpClass, GetTransform(), this);
	if (CS_Obj.ID)
	{
		CSharp::ExectuteFunctionOnObject(CS_Obj, "Begin");
		if (GetName() == "CSharpObject")
		{
			SetName(CSharpClass);
		}
	}
}

void CSharpObject::OnPropertySet()
{
	if (OldCSharpClass != CSharpClass)
	{
		Reload();
	}
}

void CSharpObject::LoadClass(std::string ClassName)
{
	CSharpClass = ClassName;
	Reload();
}

#endif