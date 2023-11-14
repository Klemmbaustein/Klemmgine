#include "WorldObject.h"
#include "Components/Component.h"
#include <sstream>
#include <Engine/Log.h>
#include <Engine/Scene.h>
#include <Networking/Networking.h>
#include <Networking/Server.h>
#include <Networking/Client.h>
#include <Networking/NetworkEvent.h>
#include <Engine/Utility/StringUtility.h>

namespace Objects
{
	std::set<WorldObject*> ObjectsToDestroy;
	std::vector<WorldObject*> AllObjects;
	std::vector<WorldObject*> GetAllObjectsWithID(uint32_t ID)
	{
		std::vector<WorldObject*> FoundObjects;
		for (WorldObject* o : Objects::AllObjects)
		{
			if (o->GetObjectDescription().ID == ID && ObjectsToDestroy.find(o) == ObjectsToDestroy.end())
			{
				FoundObjects.push_back(o);
			}
		}
		return FoundObjects;
	}
}

void WorldObject::_CallEvent(NetEvent::NetEventFunction Function, std::vector<std::string> Arguments)
{
	for (const auto& i : NetEvents)
	{
		if (i.Function == Function)
		{
			i.Invoke(Arguments);
			return;
		}
	}
	Log::Print("[Net]: Could not invoke Event", Log::LogColor::Yellow);
}

WorldObject::WorldObject(ObjectDescription _d)
{
	TypeName = _d.Name;
	TypeID = _d.ID;
}

WorldObject::~WorldObject()
{
}

WorldObject* WorldObject::Start(std::string ObjectName, Transform Transform, uint64_t NetID)
{
#if !EDITOR
	if ((!Client::GetIsConnected() && !Server::IsServer()) || (!GetIsReplicated() || NetID != Networking::ServerID))
#endif
	{
		this->NetID = NetID;
		Name = ObjectName;
		CurrentScene = Scene::CurrentScene;
		Objects::AllObjects.push_back(this);
		SetTransform(Transform);
		Begin();
	}
#if !EDITOR && SERVER
	else if (NetID == Networking::ServerID)
	{
		return Networking::SpawnReplicatedObjectFromID(TypeID, Transform);
	}
#elif !EDITOR
	else
	{
		delete this;
		return nullptr;
	}
#endif

	return this;
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

bool WorldObject::GetIsReplicated()
{
	return false;
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
	return (int)Components.size() - 1;
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
	for (size_t i = 0; i < Components.size(); i++)
	{
		Components.at(i)->Tick();
	}
}

void WorldObject::AddEditorProperty(Property p)
{
	p.PType = Property::PropertyType::EditorProperty;
	Properties.push_back(p);
}

void WorldObject::AddNetProperty(Property p, Property::NetOwner Owner)
{
	p.PType = Property::PropertyType::NetProperty;
	p.PropertyOwner = Owner;
	Properties.push_back(p);
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
	for (Property p : Properties)
	{
		if (p.PType != Property::PropertyType::EditorProperty)
		{
			continue;
		}

		StrUtil::ReplaceChar(p.Name, '#', "\\#");
		StrUtil::ReplaceChar(p.Name, ';', "\\;");

		OutProperties << p.Name << ";" << p.Type << ";";
		OutProperties << p.ValueToString();
		OutProperties << "#";
	}
	return OutProperties.str();
}

void WorldObject::LoadProperties(std::string in)
{
	int i = 0;
	Property CurrentProperty = Property("", Type::Float, nullptr);
	std::string current;
	char prev = 0;
	for (char c : in)
	{
		if ((c != ';' && c != '#') || prev == '\\')
		{
			if (prev == '\\' && c == '\\')
			{
				prev = 0;
			}
			if (prev == '\\')
			{
				current.pop_back();
			}
			current = current + c;
			prev = c;
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
				for (Property& p : Properties)
				{
					if (p.PType != Property::PropertyType::EditorProperty)
					{
						continue;
					}
					if (p.Name == CurrentProperty.Name)
					{
						switch (CurrentProperty.Type)
						{
						case Type::Float:
							*((float*)p.Data) = std::stof(current);
							break;
						case Type::Int:
							*((int*)p.Data) = std::stoi(current);
							break;
						case Type::String:
							*((std::string*)p.Data) = (current);
							break;
						case Type::Vector3Color:
						case Type::Vector3:
							*((Vector3*)p.Data) = Vector3::stov(current);
							break;
						case Type::Bool:
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
	for (auto* o : Objects::ObjectsToDestroy)
	{
#if !EDITOR
		if (Client::GetIsConnected() && o->GetIsReplicated())
		{
			if (Client::GetClientID() == o->NetOwner || Client::GetClientID() == Networking::ServerID)
			{
#if !SERVER
				NetworkEvent::TriggerNetworkEvent("__destr", {}, o, Networking::ServerID);
#else
				Server::HandleDestroyObject(o);
#endif
			}
		}
#endif
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

void WorldObject::SetNetOwner(int64_t NewNetID)
{
#if SERVER
	Server::SetObjNetOwner(this, NewNetID);
#endif
}

std::string WorldObject::Property::ValueToString()
{
	switch (Type)
	{
	case Type::Float:
		return std::to_string(*(float*)Data);
	case Type::Int:
		return std::to_string(*((int*)Data));
	case Type::String:
		return *((std::string*)Data);
	case Type::Vector3Color:
	case Type::Vector3:
		return (*(Vector3*)Data).ToString();
	case Type::Bool:
		return std::to_string(*((bool*)Data));
	default:
		return std::string();
	}
}

void WorldObject::NetEvent::Invoke(std::vector<std::string> Arguments) const
{
#if !EDITOR
	switch (Type)
	{
	case WorldObject::NetEvent::EventType::Server:
		if (Client::GetClientID() == Parent->NetOwner)
		{
			NetworkEvent::TriggerNetworkEvent(Name, Arguments, Parent, Networking::ServerID);
		}
		else
		{
			Log::PrintMultiLine("Attempted to invoke NetEvent '" + Name + "' on " + Networking::ClientIDToString(Client::GetClientID()) + "\n"
				"but the event can only be called from the owning client (client " + Networking::ClientIDToString(Parent->NetOwner) + ")",
				Log::LogColor::Yellow,
				"[Net]: ");
		}
		break;
	case WorldObject::NetEvent::EventType::Owner:
		if (Client::GetClientID() == Networking::ServerID)
		{
			NetworkEvent::TriggerNetworkEvent(Name, Arguments, Parent, Parent->NetOwner);
		}
		else
		{
			Log::PrintMultiLine("Attempted to invoke NetEvent '" + Name + "' on " + Networking::ClientIDToString(Client::GetClientID()) + "\n"
				"but the event can only be called from the server",
				Log::LogColor::Yellow,
				"[Net]: ");
		}
		break;
	case WorldObject::NetEvent::EventType::Clients:
		if (Client::GetClientID() == Networking::ServerID)
		{
			for (const auto& i : Server::GetClients())
			{
				NetworkEvent::TriggerNetworkEvent(Name, Arguments, Parent, i.ID);
			}
		}
		else
		{
			Log::PrintMultiLine("Attempted to invoke NetEvent '" + Name + "' on " + Networking::ClientIDToString(Client::GetClientID()) + "\n"
				"but the event can only be called from the server",
				Log::LogColor::Yellow,
				"[Net]: ");
		}
		break;
	default:
		break;
	}
#endif
}
