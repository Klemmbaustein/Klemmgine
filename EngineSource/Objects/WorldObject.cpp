#include "WorldObject.h"
#include "Components/Component.h"
#include <sstream>
#include <Engine/Log.h>

namespace Objects
{
	std::set<WorldObject*> ObjectsToDestroy;
}

WorldObject::WorldObject(ObjectDescription _d)
{
	TypeName = _d.Name;
	TypeID = _d.ID;
}

WorldObject::~WorldObject()
{
}

void WorldObject::Start(std::string ObjectName, Transform Transform)
{
	Name = ObjectName;
	Objects::AllObjects.push_back(this);
	SetTransform(Transform);
	Begin();
}

void WorldObject::Destroy()
{
}

void WorldObject::Tick()
{
}

void WorldObject::Begin()
{
}

void WorldObject::SetTransform(Transform NewTransform)
{
	ObjectTransform = NewTransform;
}

Transform& WorldObject::GetTransform()
{
	return ObjectTransform;
}

int WorldObject::Attach(Component* NewComponent)
{
	Components.push_back(NewComponent);
	ComponentModifier::SetParent(NewComponent, this);
	NewComponent->Begin();
	return Components.size() - 1;
}

void WorldObject::SetName(std::string Name)
{
	this->Name = Name;
}

std::string WorldObject::GetName()
{
	return Name;
}

ObjectDescription WorldObject::GetObjectDescription()
{
	return ObjectDescription(TypeName, TypeID);
}

void WorldObject::TickComponents()
{
	for (int i = 0; i < Components.size(); i++)
	{
		Components.at(i)->Tick();
	}
}


std::string WorldObject::Serialize()
{
	return "";
}


void WorldObject::Detach(Component* C)
{
	for (int i = 0; i < Components.size(); i++)
	{
		if (Components[i] == C)
		{
			Components[i]->Destroy();
			delete Components[i];
			Components.erase(Components.begin() + i);
		}
	}
}

void WorldObject::Deserialize(std::string SerializedObject)
{
}

void WorldObject::OnPropertySet()
{
}

std::string WorldObject::GetPropertiesAsString()
{
	std::stringstream OutProperties;
	for (Objects::Property p : Properties)
	{
		OutProperties << p.Name << ";" << p.Type << ";";
		switch (p.Type)
		{
		case Type::E_FLOAT:
			OutProperties << std::to_string(*(float*)p.Data);
			break;
		case Type::E_INT:
			OutProperties << std::to_string(*((int*)p.Data));
			break;
		case Type::E_STRING:
			OutProperties << *((std::string*)p.Data);
			break;
		case Type::E_VECTOR3_COLOR:
		case Type::E_VECTOR3:
			OutProperties << (*(Vector3*)p.Data).ToString();
			break;
		case Type::E_BOOL:
			OutProperties << std::to_string(*((bool*)p.Data));
			break;
		default:
			break;
		}
		OutProperties << "#";
	}
	return OutProperties.str();
}

void WorldObject::LoadProperties(std::string in)
{
	int i = 0;
	Objects::Property CurrentProperty = Objects::Property("", Type::E_FLOAT, nullptr);
	std::string current;
	for (char c : in)
	{
		if (c != ';' && c != '#')
		{
			current = current + c;
		}
		else
		{

			switch (i)
			{
			case 0:
				CurrentProperty.Name = current;
				break;
			case 1:
				CurrentProperty.Type = (Type::TypeEnum)std::stoi(current);
				break;
			case 2:
				if (current.empty())
				{
					current.clear();
					i = 0;
					continue;
				}
				for (Objects::Property p : Properties)
				{
					if (p.Name == CurrentProperty.Name)
					{
						switch (CurrentProperty.Type)
						{
						case Type::E_FLOAT:
							*((float*)p.Data) = std::stof(current);
							break;
						case Type::E_INT:
							*((int*)p.Data) = std::stoi(current);
							break;
						case Type::E_STRING:
							*((std::string*)p.Data) = (current);
							break;
						case Type::E_VECTOR3_COLOR:
						case Type::E_VECTOR3:
							*((Vector3*)p.Data) = Vector3::stov(current);
							break;
						case Type::E_BOOL:
							*((bool*)p.Data) = std::stoi(current);
							break;
						default:
							break;
						}
						break;
					}
				}
				i = -1;
				break;
			default:
				break;
			}
			current.clear();
			i++;
		}
	}
}

void WorldObject::DestroyMarkedObjects()
{
	for(auto* o : Objects::ObjectsToDestroy)
	{
		o->Destroy();
		for (Component* LoopComponent : o->GetComponents())
		{
			LoopComponent->Destroy();
			delete LoopComponent;
		}
		for (int j = 0; j < Objects::AllObjects.size(); j++)
		{
			if (Objects::AllObjects[j] == o)
			{
				Objects::AllObjects.erase(Objects::AllObjects.begin() + j);
				break;
			}
		}
		delete o;
	}

	Objects::ObjectsToDestroy.clear();
}