#include "Engine/File/Save.h"
#include <fstream>
#include <filesystem>
#include <Engine/Log.h>
#include <sstream>
#include <iostream>

class InvalidTypeException : public std::exception
{
public:
	const char* what() const override
	{
		return "Loaded invalid type from file.";
	};
};

SaveGame::SaveGame(std::string SaveName, std::string Extension, bool InSaveFolder)
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
		SaveName.append("." + Extension);
	}
	OpenedSave = SaveName;

	//if a save does not exist yet, it will be created on deconstruction.
	if (std::filesystem::exists(SaveName))
	{
		IsNew = std::filesystem::is_empty(SaveName);
		std::ifstream InFile = std::ifstream(SaveName, std::ios::in);
		char CurrentBuff[100];

		//iterate through all lines which (hopefully) contain save values
		while (!InFile.eof())
		{

			Type::TypeEnum CurrentType = Type::Null;
			std::string CurrentName = "";
			std::string Value = "";

			std::string CurrentLine;
			InFile.getline(CurrentBuff, 100);
			CurrentLine = CurrentBuff;

			if (CurrentLine.substr(0, 2) == "//") continue;


			std::stringstream CurrentLineStream = std::stringstream(CurrentLine);

			//if the current line is empty, we ignore it
			if (!CurrentLine.empty())
			{

				std::string Type;
				CurrentLineStream >> Type;
				for (unsigned int i = 0; i < 8; i++)
				{
					if (Type::Types[i] == Type)
					{
						CurrentType = (Type::TypeEnum)((int)i);
					}
				}
				if (CurrentType == Type::Null)
				{
					Log::Print("Error reading save file: " + Type + " is not a valid type (" + CurrentLine + ")", Vector3(1, 0, 0));
				}
				std::string Equals;

				CurrentLineStream >> CurrentName;
				CurrentLineStream >> Equals;

				if (Equals != "=")
				{
					Log::Print("Error reading save file: expected = sign (" + CurrentLine + ")", Vector3(1, 0, 0));
				}
				//the rest of the stream is the value of the save item
				while (!CurrentLineStream.eof())
				{
					std::string ValueToAppend;
					CurrentLineStream >> ValueToAppend;
					Value.append(ValueToAppend + " ");
				}
				const auto strBegin = Value.find_first_not_of(" ");

				const auto strEnd = Value.find_last_not_of(" ");
				const auto strRange = strEnd - strBegin + 1;
				if (strBegin != std::string::npos)
				{
					Value = Value.substr(strBegin, strRange);
				}
				else
				{
					Value = "";
				}
				Properties.insert(std::pair(CurrentName, SaveProperty(CurrentName, Value, CurrentType)));
			}
		}
		InFile.close();
	}
}

SaveGame::SaveProperty SaveGame::GetProperty(std::string Name)
{
	auto FoundIndex = Properties.find(Name);
	if (FoundIndex != Properties.end())
	{
		return Properties.find(Name)->second;
	}
	return SaveProperty();
}

void SaveGame::SetProperty(SaveProperty S)
{
	if (Properties.find(S.Name) != Properties.end())
	{
		Properties[S.Name] = S;
		return;
	}
	else
	{
		Properties.insert(std::pair(S.Name, S));
	}
}

SaveGame::SaveProperty SaveGame::GetPropertyOfType(std::string Name, Type::TypeEnum PropertyType)
{
	auto P = GetProperty(Name);
	if (P.Type != PropertyType)
	{
		Log::Print("Incorrect property - Expected " + Type::Types[PropertyType] + ", found: " + Type::Types[P.Type], Log::LogColor::Red);
		throw InvalidTypeException();
	}
	return P;
}

int SaveGame::GetInt(std::string Name)
{
	try
	{
		return std::stoi(GetPropertyOfType(Name, Type::Int).Value);
	}
	catch (std::exception&)
	{
		return 0;
	}
}

bool SaveGame::GetBool(std::string Name)
{
	try
	{
		return std::stoi(GetPropertyOfType(Name, Type::Bool).Value);
	}
	catch (std::exception&)
	{
		return 0;
	}
}

std::string SaveGame::GetString(std::string Name)
{
	try
	{
		return GetPropertyOfType(Name, Type::String).Value;
	}
	catch (std::exception&)
	{
		return "";
	}
}

float SaveGame::GetFloat(std::string Name)
{
	try
	{
		return std::stof(GetPropertyOfType(Name, Type::Float).Value);
	}
	catch (std::exception&)
	{
		return 0;
	}
}


Vector3 SaveGame::GetVector(std::string Name)
{
	try
	{
		return Vector3::stov(GetPropertyOfType(Name, Type::Vector3).Value);
	}
	catch (std::exception&)
	{
		return 0;
	}
}

SaveGame::~SaveGame()
{
	std::ofstream OutFile = std::ofstream(OpenedSave, std::ios::out);
	// loop through all the properties and write them to the "OpenedSave" file
	for (const auto& p : Properties)
	{
		OutFile << Type::Types[p.second.Type];
		OutFile << " ";
		OutFile << p.second.Name;
		OutFile << " = ";
		OutFile << p.second.Value;
		OutFile << std::endl;

	}
	OutFile.close();
}

bool SaveGame::SaveGameIsNew()
{
	return IsNew;
}

std::map<std::string, SaveGame::SaveProperty> SaveGame::GetProperties()
{
	return Properties;
}

SaveGame::SaveProperty::SaveProperty(std::string Name, std::string Value, Type::TypeEnum Type)
{
	this->Name = Name;
	this->Value = Value;
	this->Type = Type;
}
