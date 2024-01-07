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
#include <Objects/CSharpObject.h>

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
	if ((!Client::GetIsConnected() && !Server::IsServer()) || (!GetIsReplicated() || NetID != UINT64_MAX))
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

void WorldObject::Update()
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

ObjectDescription WorldObject::GetObjectDescription()
{
	return ObjectDescription(TypeName, TypeID);
}

void WorldObject::UpdateComponents()
{
	for (size_t i = 0; i < Components.size(); i++)
	{
		Components.at(i)->Update();
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
		if (p.PType != Property::PropertyType::EditorProperty && p.PType != Property::PropertyType::CSharpProperty)
		{
			continue;
		}

		StrUtil::ReplaceChar(p.Name, '#', "\\#");
		StrUtil::ReplaceChar(p.Name, ';', "\\;");

		if (p.PType == Property::PropertyType::CSharpProperty)
		{
			OutProperties << "@csharp_";
		}
		OutProperties << p.Name << ";" << p.Type << ";";
		OutProperties << p.ValueToString(this);
		OutProperties << "#";
	}
	return OutProperties.str();
}

template<typename T>
static void AssignStringToPtr(Type::TypeEnum t, std::string str, void* ptr, T (*FromString)(std::string val))
{
	if (t & Type::List)
	{
		auto vec = (std::vector<T>*)ptr;
		vec->clear();
		std::string val;
		for (unsigned char i : str)
		{
			if (i != 0xff)
			{
				val.append({ (char)i });
			}
			else
			{
				vec->push_back(FromString(val));
				val.clear();
			}
		}
		if (!val.empty())
		{
			vec->push_back(FromString(val));
		}
		return;
	}
	*(T*)ptr = FromString(str);
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
				try
				{
					CurrentProperty.Type = (Type::TypeEnum)std::stoi(current);
				}
				catch (std::exception)
				{
					return;
				}
				break;
			case 2:
				if (current.empty())
				{
					current.clear();
					i = 0;
					continue;
				}
				{
					std::string CSharpSeperator = "@csharp";
					if (CurrentProperty.Name.substr(0, CSharpSeperator.size()) == CSharpSeperator)
					{
						Property p = Property(CurrentProperty.Name, CurrentProperty.Type, nullptr);
						p.PType = Property::PropertyType::CSharpProperty;
						p.ValueString = current;
						Properties.push_back(p);
					}
				}
				for (Property& p : Properties)
				{
					if (p.PType != Property::PropertyType::EditorProperty)
					{
						continue;
					}
					if (p.Name == CurrentProperty.Name)
					{
						switch ((Type::TypeEnum)(CurrentProperty.Type &  ~Type::List))
						{
						case Type::Float:
							AssignStringToPtr<float>(p.Type, current, p.Data, [](std::string val){return std::stof(val);});
							break;
						case Type::Int:
							AssignStringToPtr<int>(p.Type, current, p.Data, [](std::string val) {return std::stoi(val); });
							break;
						case Type::String:
							AssignStringToPtr<std::string>(p.Type, current, p.Data, [](std::string val) {return val; });
							break;
						case Type::Vector3Color:
						case Type::Vector3Rotation:
						case Type::Vector3:
							AssignStringToPtr<Vector3>(p.Type, current, p.Data, [](std::string val) {return Vector3::FromString(val); });
							break;
						case Type::Bool:
							AssignStringToPtr<bool>(p.Type, current, p.Data, [](std::string val) {return (bool)std::stoi(val); });
							break;
						default:
							Log::Print(std::to_string(CurrentProperty.Type ^ Type::List));
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

template<typename T>
static std::string ListToString(void* Data, std::string(*ToString)(T Data))
{
	auto& vec = *(std::vector<T>*)(Data);

	std::string val;

	for (auto i : vec)
	{
		val.append(ToString(i) + std::string({(char)0xff}));
	}
	if (!val.empty())
	{
		val.pop_back();
	}
	return val;
}

std::string WorldObject::Property::ValueToString(WorldObject* Context)
{
#if ENGINE_CSHARP
	if (PType == PropertyType::CSharpProperty)
	{
		return static_cast<CSharpObject*>(Context)->GetProperty(Name.substr(Name.find_first_of(":") + 1));
	}
#endif
	if (Type & Type::List)
	{
		Type = (Type::TypeEnum)(Type ^ Type::List);

		switch (Type)
		{
		case Type::Vector3Color:
		case Type::Vector3Rotation:
		case Type::Vector3:
		{
			return ListToString<Vector3>(Data, [](Vector3 Data){
				return Data.ToString();
				});
		}
		case Type::Float:
		{
			return ListToString<float>(Data, [](float Data) {
				return std::to_string(Data);
				});
		}
		case Type::Int:
			return ListToString<int>(Data, [](int Data) {
				return std::to_string(Data);
				});
		case Type::String:
			return ListToString<std::string>(Data, [](std::string Data) {
				return Data;
				});
		case Type::Byte:
			return ListToString<uint8_t>(Data, [](uint8_t Data) {
				return std::to_string(Data);
				});
		case Type::Bool:
			return ListToString<bool>(Data, [](bool Data) {
				return std::to_string(Data);
				});
		default:
			break;
		}
		return std::string();
	}

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
		break;
	}
	return std::string();
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
