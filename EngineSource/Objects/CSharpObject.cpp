#ifdef ENGINE_CSHARP
#include "CSharpObject.h"
#include <Engine/Log.h>

void CSharpObject::Begin()
{
	Properties.push_back(Objects::Property("Object class", Type::String, &CSharpClass));
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
	Transform NewTransform;
	NewTransform.Location = CSharp::GetObjectVectorField(CS_Obj, "Position");
	NewTransform.Rotation = CSharp::GetObjectVectorField(CS_Obj, "Rotation");
	NewTransform.Scale = CSharp::GetObjectVectorField(CS_Obj, "Scale");
	if (NewTransform != CSharpTransform)
	{
		SetTransform(NewTransform);
		CSharpTransform = NewTransform;
	}
#else
	CSharp::SetObjectVectorField(CS_Obj, "Position", GetTransform().Location);
	CSharp::SetObjectVectorField(CS_Obj, "Rotation", GetTransform().Rotation);
	CSharp::SetObjectVectorField(CS_Obj, "Scale", GetTransform().Scale);
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