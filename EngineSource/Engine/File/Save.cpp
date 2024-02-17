#include "Engine/File/Save.h"
#include <fstream>
#include <filesystem>
#include <Engine/Log.h>
#include <sstream>
#include <iostream>


SaveGame::SaveGame(std::string SaveName, std::string Extension, bool InSaveFolder, bool ShouldSaveOnClose)
{
	this->ShouldSave = ShouldSaveOnClose;
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
	IsNew = !std::filesystem::exists(SaveName) || std::filesystem::is_empty(SaveName);

	//if the file does not exist yet, it will be created on deconstruction.
	if (IsNew)
	{
		return;
	}
	std::ifstream InFile = std::ifstream(SaveName, std::ios::in);
	std::string NewLine;

	SaveProperty NewProperty;

	bool InVec = false;
	bool InString = false;
	bool NewLineIsString = false;

	uint8_t ReadingState = 0;

	size_t LineCounter = 1;
	std::string CurrentLine;

	bool IsReadingComment = false;

	while (true)
	{
		char NewChar;
		InFile.read(&NewChar, 1);

		CurrentLine.append({ NewChar });
		
		if (NewChar == '\n')
		{
			LineCounter++;
			IsReadingComment = false;
		}
		else if (IsReadingComment)
		{
			continue;
		}

		if ((NewChar == ' ' || NewChar == '\n' || NewChar == '	' || InFile.eof())
			&& !InVec
			&& !InString)
		{
			if (!NewLine.empty() || NewLineIsString)
			{
				NewLineIsString = false;
				switch (ReadingState++)
				{
				case 0:
					for (size_t i = 0; i < NativeType::TypeStrings.size(); i++)
					{
						if (NativeType::TypeStrings[i] == NewLine)
						{
							NewProperty.NativeType = (NativeType::NativeType)i;
						}
					}
					if (NewProperty.NativeType == NativeType::Null)
					{
						CurrentLine.pop_back();
						Log::PrintMultiLine("Loading error: expected a valid type:\n"
							+ CurrentLine + " <- HERE\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^",
							Log::LogColor::Red,
							"[Error: " + SaveName + "]: ");
						Properties.clear();
						return;
					}
					break;
				case 1:
					NewProperty.Name = NewLine;
					break;
				case 2:
					if (NewLine != "=")
					{
						CurrentLine.pop_back();
						Log::PrintMultiLine("Loading error: expected a '=':\n" 
							+ CurrentLine + " <- HERE\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^",
							Log::LogColor::Red, 
							"[Error: " + SaveName + "]: ");
						Properties.clear();
						return;
					}
					break;
				case 3:
					NewProperty.Value = NewLine;
					ReadingState = 0;
					Properties.insert(std::pair(NewProperty.Name, NewProperty));
					NewProperty = SaveProperty();
					CurrentLine.clear();
					break;
				default:
					break;
				}
			}

			NewLine.clear();

			if (InFile.eof())
			{
				break;
			}
			continue;
		}
		else if (InFile.eof())
		{
			CurrentLine.pop_back();
			if (InString)
			{
				Log::PrintMultiLine("Loading error: expected closing '\"':\n"
					+ CurrentLine + " <- HERE\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^",
					Log::LogColor::Red,
					"[Error: " + SaveName + "]: ");
			}
			else
			{
				Log::PrintMultiLine("Loading error: expected closing '>':\n"
					+ CurrentLine + " <- HERE\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^",
					Log::LogColor::Red,
					"[Error: " + SaveName + "]: ");
			}
			Properties.clear();
			return;
		}

		if (!InString && NewLine.size() && NewLine[NewLine.size() - 1] == '/' && NewChar == '/')
		{
			IsReadingComment = true;
			NewLine.clear();
			continue;
		}

		if (!InString && !InVec)
		{
			if (NewLine.empty() && NewChar == '"')
			{
				NewLineIsString = true;
				InString = true;
				continue;
			}
			if (NewLine.empty() && NewChar == '<')
			{
				InVec = true;
				continue;
			}
		}
		else
		{
			if (InString && NewChar == '"' && (!NewLine.size() || NewLine[NewLine.size() - 1] != '\\'))
			{
				NewLineIsString = true;
				InString = false;
				continue;
			}
			if (InVec && NewChar == '>')
			{
				InVec = false;
				continue;
			}
		}

		NewLine.append({ NewChar });
	}

}

SaveGame::SaveProperty SaveGame::GetProperty(std::string Name) const
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

SaveGame::SaveProperty SaveGame::GetPropertyOfType(std::string Name, NativeType::NativeType PropertyType) const
{
	auto P = GetProperty(Name);
	if (P.NativeType != PropertyType)
	{
		Log::Print("Incorrect property - Expected " + NativeType::TypeStrings[PropertyType] + ", found: " + NativeType::TypeStrings[P.NativeType], Log::LogColor::Red);
	}
	return P;
}

int SaveGame::GetInt(std::string Name) const
{
	try
	{
		return std::stoi(GetPropertyOfType(Name, NativeType::Int).Value);
	}
	catch (std::exception&)
	{
		return 0;
	}
}

bool SaveGame::GetBool(std::string Name) const
{
	try
	{
		return std::stoi(GetPropertyOfType(Name, NativeType::Bool).Value);
	}
	catch (std::exception&)
	{
		return 0;
	}
}

std::string SaveGame::GetString(std::string Name) const
{
	try
	{
		return GetPropertyOfType(Name, NativeType::String).Value;
	}
	catch (std::exception&)
	{
		return "";
	}
}

float SaveGame::GetFloat(std::string Name) const
{
	try
	{
		return std::stof(GetPropertyOfType(Name, NativeType::Float).Value);
	}
	catch (std::exception&)
	{
		return 0;
	}
}


Vector3 SaveGame::GetVector(std::string Name) const
{
	try
	{
		return Vector3::FromString(GetPropertyOfType(Name, NativeType::Vector3).Value);
	}
	catch (std::exception&)
	{
		return 0;
	}
}

SaveGame::~SaveGame()
{
	if (!ShouldSave)
	{
		return;
	}
	std::ofstream OutFile = std::ofstream(OpenedSave, std::ios::out);
	// loop through all the properties and write them to the "OpenedSave" file
	for (const auto& p : Properties)
	{
		OutFile << NativeType::TypeStrings[p.second.NativeType];
		OutFile << " ";
		OutFile << p.second.Name;
		OutFile << " = ";
		switch (p.second.NativeType)
		{
		case NativeType::Int:
		case NativeType::Float:
		case NativeType::Byte:
		case NativeType::Bool:
			OutFile << p.second.Value;
			break;
		case NativeType::String:
			OutFile << "\"" << p.second.Value << "\"";
			break;
		case NativeType::Vector3:
		case NativeType::Vector3Color:
		case NativeType::Vector3Rotation:
			OutFile << "<" << p.second.Value << ">";
			break;
		default:
			break;
		}
		OutFile << std::endl;

	}
	OutFile.close();
}

bool SaveGame::SaveGameIsNew() const
{
	return IsNew;
}

std::map<std::string, SaveGame::SaveProperty> SaveGame::GetProperties() const
{
	return Properties;
}

SaveGame::SaveProperty::SaveProperty(std::string Name, std::string Value, NativeType::NativeType NativeType)
{
	this->Name = Name;
	this->Value = Value;
	this->NativeType = NativeType;
}
