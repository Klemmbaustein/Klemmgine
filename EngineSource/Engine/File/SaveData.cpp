#include "Engine/File/SaveData.h"
#include <fstream>
#include <filesystem>
#include <Engine/Log.h>
#include <sstream>
#include <iostream>
#include <Engine/Utility/FileUtility.h>

SaveData::SaveData()
{
	ShouldSave = false;
}

SaveData::SaveData(std::string SaveName, std::string Extension, bool InSaveFolder, bool ShouldSaveOnClose)
{
	if (InSaveFolder)
	{
		if (!std::filesystem::exists("Saves/"))
		{
			std::filesystem::create_directory("Saves/");
		}
		SaveName = "Saves/" + SaveName + "." + Extension;
	}
	else
	{
		if (!Extension.empty())
		{
			SaveName.append("." + Extension);
		}
	}

	bool FileIsNew = !std::filesystem::exists(SaveName) || std::filesystem::is_empty(SaveName);
	//if the file does not exist yet, it will be created on deconstruction.
	if (FileIsNew)
	{
		ShouldSave = ShouldSaveOnClose;
		OpenedSave = SaveName;
		IsNew = FileIsNew;
		return;
	}
	std::ifstream InFile = std::ifstream(SaveName, std::ios::in);
	*this = ParseString(std::string(
		(std::istreambuf_iterator<char>(InFile)),
		(std::istreambuf_iterator<char>())
	));
	ShouldSave = ShouldSaveOnClose;
	OpenedSave = SaveName;
	IsNew = FileIsNew;
}

SaveData SaveData::ParseString(std::string Str)
{
	SaveData OutData;
	std::string CurrentString;
	
	int ParseStage = 0;
	Field NewField;
	bool ReadString = false;
	bool InString = false;
	bool ReadBrackets = false;
	size_t BracketDepth = 0;

	char Last = 0;
	bool InComment = false;
	Str.append(" ");

	for (char c : Str)
	{
		if (c == '}' && !InString)
		{
			ReadBrackets = true;
			if (BracketDepth == 0)
			{
				Error("Unexpected '}'");
			}
			BracketDepth--;
			if (BracketDepth == 0)
			{
				continue;
			}
		}

		if (InComment && c == '\n')
		{
			InComment = false;
		}
		if (InComment)
		{
			continue;
		}

		switch (c)
		{
		case '{':
			if (BracketDepth)
			{
				CurrentString.push_back(c);
			}
			if (!InString)
			{
				ReadBrackets = true;
				BracketDepth++;
			}
			break;
		case '<':
		case '>':
		case '"':
			if (BracketDepth)
			{
				CurrentString.push_back(c);
			}
			if (!CurrentString.empty() && !InString && !BracketDepth)
			{
				Error("Unexpected '\"'");
			}
			InString = !InString;
			ReadString = true;
			break;
		case '=':
			if (BracketDepth > 0)
			{
				CurrentString.push_back(c);
				break;
			}
			if (ParseStage != 2)
			{
				Error("Parse error: unexpected '='");
			}
			else
			{
				ParseStage++;
			}
			break;
		case '/':
			if (Last == '/' && !InString)
			{
				InComment = true;
			}
			[[fallthrough]];
		case '\n':
		case ' ':
		case '\t':
			if (BracketDepth > 0)
			{
				CurrentString.push_back(c);
				break;
			}
			if (CurrentString.empty() && !ReadString)
			{
				break;
			}
			if (InString)
			{
				CurrentString.push_back(c);
				break;
			}

			ReadString = false;

			switch (ParseStage)
			{
			case 0:
				for (NativeType::NativeType i = (NativeType::NativeType)0; i < NativeType::TypeStrings.size(); i = NativeType::NativeType(i + 1))
				{
					if (NativeType::TypeStrings[i] == CurrentString)
					{
						NewField.Type = i;
					}
				}

				if (NewField.Type == NativeType::Null)
				{
					Error("Parse error: expected type, got '" + CurrentString + "'");
				}

				ParseStage++;
				break;
			case 1:
				NewField.Name = CurrentString;

				ParseStage++;
				break;
			case 2:
				if (CurrentString != "=")
				{
					Error("Parse error: expected '=', got '" + CurrentString + "'");
				}

				ParseStage++;
				break;
			case 3:
				if (!ReadBrackets)
				{
					NewField.Data = CurrentString;
				}
				else
				{
					auto Data = ParseString(CurrentString);
					NewField.Children = Data.Fields;
				}
				OutData.SetField(NewField);
				NewField = Field();
				ParseStage = 0;
				ReadBrackets = false;
				break;
			default:
				break;
			}
			CurrentString.clear();
			break;
			[[fallthrough]];
		default:
			CurrentString.push_back(c);
			break;
		}
		Last = c;
	}

	if (BracketDepth != 0)
	{
		std::cout << "error" << std::endl;
	}

	return OutData;
}

std::string SaveData::SerializeString() const
{
	std::string str;
	for (const auto& f : Fields)
	{
		str.append(f.Serialize(0) + "\n");
	}
	return str;
}

SaveData::Field& SaveData::GetField(std::string Name)
{
	for (SaveData::Field& i : Fields)
	{
		if (i.Name == Name)
		{
			return i;
		}
	}
	throw std::exception();
}

SaveData::Field SaveData::GetField(std::string Name) const
{
	for (const SaveData::Field& i : Fields)
	{
		if (i.Name == Name)
		{
			return i;
		}
	}
	return Field();
}

bool SaveData::HasField(std::string Name)
{
	for (auto& i : Fields)
	{
		if (i.Name == Name)
		{
			return true;
		}
	}
	return false;
}

void SaveData::SetField(Field S)
{
	if (HasField(S.Name))
	{
		GetField(S.Name) = S;
	}
	else
	{
		Fields.push_back(S);
	}
}

SaveData::Field SaveData::GetPropertyOfType(std::string Name, NativeType::NativeType PropertyType) const
{
	auto P = GetField(Name);
	if (P.Type != PropertyType)
	{
		Log::Print("Incorrect property - Expected " + NativeType::TypeStrings[PropertyType] + ", found: " + NativeType::TypeStrings[P.Type], Log::LogColor::Red);
	}
	return P;
}

int SaveData::GetInt(std::string Name) const
{
	return GetField(Name).AsInt();
}

bool SaveData::GetBool(std::string Name) const
{
	return GetField(Name).AsBool();
}

std::string SaveData::GetString(std::string Name) const
{
	return GetField(Name).AsString();
}

float SaveData::GetFloat(std::string Name) const
{
	return GetField(Name).AsFloat();
}

Vector3 SaveData::GetVector(std::string Name) const
{
	return GetField(Name).AsVector();
}

void SaveData::ClearFields()
{
	Fields.clear();
}

SaveData::~SaveData()
{
	if (!ShouldSave)
	{
		return;
	}
	SaveToFile(OpenedSave);
}

void SaveData::SaveToFile(std::string File) const
{
	size_t Last =  File.find_last_of("/\\");

	std::string Dir = File.substr(0, Last);

	if (!Dir.empty() && Last != std::string::npos)
	{
		std::filesystem::create_directories(Dir);
	}
	std::ofstream OutFile = std::ofstream(File, std::ios::out);
	OutFile.exceptions(std::ios::failbit | std::ios::badbit);
	OutFile << SerializeString();
	OutFile.close();
}

bool SaveData::SaveGameIsNew() const
{
	return IsNew;
}

std::vector<SaveData::Field>& SaveData::GetAllFields()
{
	return Fields;
}

void SaveData::Error(std::string Message)
{
	Log::Print("[KeSv]: [Error]: " + Message, Log::LogColor::Red);
}

SaveData::Field::Field()
{
}

SaveData::Field::Field(NativeType::NativeType Type, std::string Name, std::string Data)
{
	this->Type = Type;
	this->Name = Name;
	this->Data = Data;
}

bool SaveData::Field::operator<(Field b) const
{
	return Name < b.Name;
}

std::string SaveData::Field::Serialize(size_t Depth) const
{
	std::string str;
	std::string Tabs;
	if (Depth)
	{
		Tabs.resize(Depth, '\t');
		str += Tabs;
	}
	str += NativeType::TypeStrings[Type] + " " + Name + " = ";

	if (!Children.empty())
	{
		str += "\n" + Tabs + "{\n";
		for (auto& c : Children)
		{
			str += c.Serialize(Depth + 1) + "\n";
		}
		str += Tabs + "}";
		return str;
	}

	switch (Type)
	{
	case NativeType::Int:
	case NativeType::Float:
	case NativeType::Byte:
	case NativeType::Bool:
		str += Data;
		break;
	case NativeType::GL_Texture:
	case NativeType::String:
		str += "\"" + Data + "\"";
		break;
	case NativeType::Vector3:
	case NativeType::Vector3Color:
	case NativeType::Vector3Rotation:
		str += "<" + Data + ">";
		break;
	default:
		str = "\"\"";
		break;
	}
	return str;
}

SaveData::Field& SaveData::Field::At(std::string Name)
{
	for (SaveData::Field& i : Children)
	{
		if (i.Name == Name)
		{
			return i;
		}
	}
	throw std::exception();
}

bool SaveData::Field::Contains(std::string Name)
{
	for (SaveData::Field& i : Children)
	{
		if (i.Name == Name)
		{
			return true;
		}
	}
	return false;
}

int SaveData::Field::AsInt() const
{
	if (Type != NativeType::Int)
	{
		return 0;
	}

	try
	{
		return std::stoi(Data);
	}
	catch (std::exception&)
	{
		return 0;
	}
}

bool SaveData::Field::AsBool() const
{
	if (Type != NativeType::Bool && Type != NativeType::Int)
	{
		return 0;
	}

	try
	{
		return std::stoi(Data);
	}
	catch (std::exception&)
	{
		return 0;
	}
}

std::string SaveData::Field::AsString() const
{
	if (Type != NativeType::String)
	{
		return "";
	}

	return Data;
}

float SaveData::Field::AsFloat() const
{
	if (Type != NativeType::Float)
	{
		return 0;
	}

	try
	{
		return std::stof(Data);
	}
	catch (std::exception&)
	{
		return 0;
	}
}

Vector3 SaveData::Field::AsVector() const
{
	if (Type != NativeType::Int)
	{
		return 0;
	}

	try
	{
		std::string str = Data;
		return Vector3::FromString(str.substr(1, str.size() - 2));
	}
	catch (std::exception&)
	{
		return 0;
	}
}
