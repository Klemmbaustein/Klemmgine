#include "ParseFile.h"
#include <fstream>
#include <algorithm>
#include <sstream>
#include <filesystem>
#include "Log.h"
#include <iostream>

// 32-bit string hash for generating the ID of an object
static uint32_t HashString(const std::string& str)
{

	uint32_t hash = 0x811c9dc5;
	uint32_t prime = 0x1000193;

	for (int i = 0; i < str.size(); ++i)
	{
		uint8_t value = str[i];
		hash = hash ^ value;
		hash *= prime;
	}

	return hash;
}

static std::string RemoveComments(std::string Code)
{
	std::stringstream StrStream = std::stringstream(Code);
	std::string OutCode;

	char last = 0;

	bool MultiLineComment = false;
	while (true)
	{
		char Buffer[8192];
		StrStream.getline(Buffer, sizeof(Buffer));

		if (StrStream.eof())
		{
			break;
		}

		for (char c : Buffer)
		{
			if (last == c && c == '/')
			{
				OutCode.pop_back();
				break;
			}

			if (last == '/' && c == '*')
			{
				MultiLineComment = true;
				OutCode.pop_back();
				last = 0;
			}

			if (last == '*' && c == '/')
			{
				MultiLineComment = false;
				continue;
			}

			if (c == 0)
			{
				OutCode.push_back('\n');
				last = 0;
				break;
			}

			last = c;

			if (!MultiLineComment)
			{
				OutCode.push_back(c);
			}
		}
	}
	return OutCode;
}

std::vector<ParseFile::Object> ParseFile::ParseFile(std::string Path)
{
	size_t ExtensionDot = Path.find_last_of(".");

	if (ExtensionDot == std::string::npos)
	{
		return {};
	}

	std::string Ext = Path.substr(ExtensionDot);

	if (Ext != ".h" && Ext != ".hpp")
	{
		return {};
	}

	std::ifstream InFile = std::ifstream(Path);
	std::string FileString = std::string((std::istreambuf_iterator<char>(InFile)),
		std::istreambuf_iterator<char>());

	std::stringstream FileStream = std::stringstream(RemoveComments(FileString));

	std::string Namespace;

	std::vector<Object> OutObjects;

	while (!FileStream.eof())
	{
		std::string NewToken;
		FileStream >> NewToken;

		if (NewToken == "namespace")
		{
			// Don't bother with namespace parsing yet.
		}

		if (NewToken == "class")
		{
			Object NewObject;
			NewObject.Path = Path;
			FileStream >> NewObject.Name;

			if (NewObject.Name.empty() || NewObject.Name[NewObject.Name.size() - 1] == ';')
			{
				continue;
			}

			NewObject.Hash = HashString(NewObject.Name);

			std::string Next;
			FileStream >> Next;

			if (Next == ":")
			{
				while (true)
				{
					std::string ParentClass;
					FileStream >> ParentClass;

					if (ParentClass.empty())
					{
						Log::Print("Syntax error", Log::MessageType::Error);
						break;
					}

					if (ParentClass != "public")
					{
						continue;
					}

					FileStream >> ParentClass;

					if (ParentClass == "virtual")
					{
						FileStream >> ParentClass;
					}

					if (ParentClass[ParentClass.size() - 1] == ',')
					{
						ParentClass.pop_back();
						NewObject.Parents.push_back(ParentClass);
						continue;
					}

					NewObject.Parents.push_back(ParentClass);

					FileStream >> Next;
					if (Next == ",")
					{
						continue;
					}

					break;
				}
			}

			Log::Print("Possible class: " + NewObject.Name);
			OutObjects.push_back(NewObject);
			
		}
	}

	return OutObjects;
}


void ParseFile::WriteToFile(std::string str, std::string File)
{
	// Check if the new file is different from the old one. If it is, don't write the new content into it.
	// VS won't rebuild everything once the BuildTool runs once if only the files that really needed to change have changed.
	if (std::filesystem::exists(File))
	{
		std::ifstream CurrentFile = std::ifstream(File);
		std::stringstream s;
		s << CurrentFile.rdbuf();
		CurrentFile.close();
		if (str == s.str())
		{
			Log::Print("Do not need to write: " + File + ". File has not changed.");
			return;
		}
	}
	Log::Print("Written: " + File + ".");
	std::ofstream out = std::ofstream(File);
	out << str;
	out.close();
}

bool ParseFile::Object::DerivesFromWorldObject(const std::vector<Object>& AllObjects) const
{
	for (auto& Parent : Parents)
	{
		if (Parent == "WorldObject")
		{
			return true;
		}
		for (auto& Obj : AllObjects)
		{
			if (Parent == Obj.Name && Obj.DerivesFromWorldObject(AllObjects))
			{
				return true;
			}
		}
	}
	return false;
}

void ParseFile::Object::WriteGeneratedHeader(std::string TargetFolder)
{
	std::stringstream OutStream;
	std::string UppercaseName = Name;
	std::transform(Name.begin(), Name.end(), UppercaseName.begin(), ::toupper);
	OutStream << "#define " << UppercaseName << "_GENERATED(Category) "
		<< Name << "() : WorldObject(ObjectDescription(\"" << Name << "\", " << std::to_string(Hash) << ")) {}\\\n"
		<< "static std::string GetCategory() { return Category; }\\\n";
	OutStream << "static uint32_t GetID() { return " << std::to_string(Hash) << ";}\n";
	WriteToFile(OutStream.str(), TargetFolder + "/" + Name + ".h");
}
