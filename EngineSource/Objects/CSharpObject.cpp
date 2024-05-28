#ifdef ENGINE_CSHARP
#include "CSharpObject.h"
#include <Engine/Log.h>
#include <Engine/Utility/StringUtility.h>
#include <map>

static std::map<std::string, NativeType::NativeType> ManagedTypes =
{
	{"String", NativeType::String},
	{"Float", NativeType::Float},
	{"Single", NativeType::Float},
	{"Vector3", NativeType::Vector3},
	{"Boolean", NativeType::Bool},
};

void CSharpObject::Begin()
{
	AddEditorProperty(Property("C#:Object class", NativeType::String, &CSharpClass));
}

void CSharpObject::Update()
{
	if (!CS_Obj.ID)
	{
		return;
	}

	CSharpInterop::CSharpSystem->ExecuteFunctionOnObject(CS_Obj, "UpdateComponents");
#if !EDITOR
	CSharpInterop::CSharpSystem->ExecuteFunctionOnObject(CS_Obj, "Update");
#endif
}

void CSharpObject::Destroy()
{
	CSharpInterop::CSharpSystem->DestroyObject(CS_Obj);
}

void CSharpObject::Reload(bool DeleteParameters)
{
	if (DeleteParameters)
	{
		Properties.clear();
		AddEditorProperty(Property("C#:Object class", NativeType::String, &CSharpClass));
	}
	OldCSharpClass = CSharpClass;
	if (CS_Obj.ID)
	{
		CSharpInterop::CSharpSystem->DestroyObject(CS_Obj);
	}
	CS_Obj = CSharpInterop::CSharpSystem->InstantiateObject(CSharpClass, GetTransform(), this);
	if (CS_Obj.ID)
	{
		auto LoadedProperties = StrUtil::SeperateString(CSharpInterop::CSharpSystem->ExecuteStringFunctionOnObject(CS_Obj, "GetEditorProperties"), ';');

		if (!DeleteParameters)
		{
			for (const Property& i : Properties)
			{
				if (i.PType == Property::PropertyType::CSharpProperty && !i.ValueString.empty())
				{
					static_cast<CSharpObject*>(this)->SetProperty(i.Name.substr(i.Name.find_last_of(":") + 1), i.ValueString);
				}
			}
		}
		CSharpInterop::CSharpSystem->ExecuteFunctionOnObject(CS_Obj, "Begin");
		size_t it = 1;
		for (const std::string& i : LoadedProperties)
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
				Log::Print("Unknown managed type: " + values[0] + " of variable " + values[1], Log::LogColor::Red);
				continue;
			}
			Property p = Property(values[1], ManagedTypes[values[0]], nullptr);
			if (Array)
			{
				p.NativeType = (NativeType::NativeType)(p.NativeType | NativeType::List);
			}
			p.PType = Property::PropertyType::CSharpProperty;

			if (DeleteParameters || Properties.size() <= it)
			{
				Properties.push_back(p);
				it++;
			}
			else
			{
				Properties[it++] = p;
			}
		}
		Properties.resize(it);
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

std::string CSharpObject::GetProperty(std::string PropertyName) const
{
	return CSharpInterop::CSharpSystem->GetPropertyOfObject(CS_Obj, PropertyName);
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
	CSharpInterop::CSharpSystem->SetPropertyOfObject(CS_Obj, PropertyName, Value);
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