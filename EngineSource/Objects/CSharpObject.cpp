#ifdef ENGINE_CSHARP
#include "CSharpObject.h"
#include <Engine/Log.h>
#include <Engine/Utility/StringUtility.h>
#include <map>

static std::map<std::string, Type::TypeEnum> ManagedTypes =
{
	{"String", Type::String},
	{"Float", Type::Float},
	{"Vector3", Type::Vector3},
	{"Boolean", Type::Bool}
};

void CSharpObject::Begin()
{
	AddEditorProperty(Property("C#:Object class", Type::String, &CSharpClass));
}

void CSharpObject::Update()
{
	if (!CS_Obj.ID)
	{
		return;
	}

	CSharp::ExectuteFunctionOnObject(CS_Obj, "UpdateComponents");
#if !EDITOR
	CSharp::ExectuteFunctionOnObject(CS_Obj, "Update");
#endif
}

void CSharpObject::Destroy()
{
	CSharp::DestroyObject(CS_Obj);
}

void CSharpObject::Reload(bool DeleteParameters)
{
	if (DeleteParameters)
	{
		Properties.clear();
		AddEditorProperty(Property("C#:Object class", Type::String, &CSharpClass));
	}
	OldCSharpClass = CSharpClass;
	if (CS_Obj.ID)
	{
		CSharp::DestroyObject(CS_Obj);
	}
	CS_Obj = CSharp::InstantiateObject(CSharpClass, GetTransform(), this);
	if (CS_Obj.ID)
	{
		auto LoadedProperties = StrUtil::SeperateString(CSharp::ExectuteStringFunctionOnObject(CS_Obj, "GetEditorProperties"), ';');

		if (!DeleteParameters)
		{
			for (auto& i : Properties)
			{
				if (i.PType == Property::PropertyType::CSharpProperty && !i.ValueString.empty())
				{
					static_cast<CSharpObject*>(this)->SetProperty(i.Name.substr(i.Name.find_last_of(":") + 1), i.ValueString);
				}
			}
		}
		CSharp::ExectuteFunctionOnObject(CS_Obj, "Begin");
		size_t it = 1;
		for (auto& i : LoadedProperties)
		{
			size_t FirstSpace = i.find_first_of(" ");
			std::vector<std::string> values = {i.substr(0, FirstSpace), i.substr(FirstSpace + 1)};
			bool Array = values[0].size() > 2 && values[0].substr(values[0].size() - 2) == "[]";
			if (Array)
			{
				values[0] = values[0].substr(0, values[0].size() - 2);
			}

			if (!ManagedTypes.contains(values[0]))
			{
				Log::Print("Unknown managed type: " + values[0], Log::LogColor::Red);
				continue;
			}
			Property p = Property(values[1], ManagedTypes[values[0]], nullptr);
			if (Array)
			{
				p.Type = (Type::TypeEnum)(p.Type | Type::List);
			}
			p.PType = Property::PropertyType::CSharpProperty;

			if (DeleteParameters || Properties.size() <= it)
			{
				Properties.push_back(p);
			}
			else
			{
				Properties[it++] = p;
			}
		}
		if (Name == "CSharpObject")
		{
			Name = CSharpClass;
		}
	}
	else if (!CSharpClass.empty())
	{
		Log::Print("Could not load C# class: " + CSharpClass, Log::LogColor::Yellow);
	}
}

std::string CSharpObject::GetProperty(std::string PropertyName)
{
	return CSharp::GetPropertyOfObject(CS_Obj, PropertyName);
}

void CSharpObject::SetProperty(std::string PropertyName, std::string Value)
{
	for (auto& i : Properties)
	{
		if (i.Name.substr(i.Name.find_last_of(":") + 1) == PropertyName)
		{
			i.ValueString = Value;
		}
	}
	return CSharp::SetPropertyOfObject(CS_Obj, PropertyName, Value);
}

void CSharpObject::OnPropertySet()
{
	if (OldCSharpClass != CSharpClass)
	{
		Reload(Properties.empty() || !OldCSharpClass.empty());
	}
}

void CSharpObject::LoadClass(std::string ClassName)
{
	CSharpClass = ClassName;
	Reload(true);
}

#endif