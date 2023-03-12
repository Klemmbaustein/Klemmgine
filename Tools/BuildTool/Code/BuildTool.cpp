#include "Log.h"
#include <vector>
#include "Exception.h"
#include <filesystem>
#include <fstream>
#include <algorithm>

struct ParseException : public Exception
{
	ParseException(std::string Text) : Exception(Text, "Parsing Error") {}
};

// 32bit string hash for gnerating the ID of an object
inline uint32_t hash_str_uint32(const std::string& str) {

	uint32_t hash = 0x811c9dc5;
	uint32_t prime = 0x1000193;

	for (int i = 0; i < str.size(); ++i) {
		uint8_t value = str[i];
		hash = hash ^ value;
		hash *= prime;
	}

	return hash;

}

struct Object
{
	std::string Name;
	std::string Path;
};

void WriteObjectList(std::vector<Object> Objects, std::string TargetFolder)
{
	std::ofstream OutStream = std::ofstream(TargetFolder + "/GENERATED_ListOfObjects.h", std::ios::out);
	Log::Print("Written: " + TargetFolder + "/GENERATED_ListOfObjects.h");
	for (unsigned int i = 0; i < Objects.size(); i++)
	{
		uint32_t ID = hash_str_uint32(Objects[i].Name);
		OutStream << "ObjectDescription(\"" << Objects[i].Name << "\", " << ID << ")," << std::endl;
	}
	OutStream.close();
}

void WriteIncludeList(std::vector<Object> Objects, std::string TargetFolder)
{
	std::ofstream OutStream = std::ofstream(TargetFolder + "/GENERATED_ObjectIncludes.h", std::ios::out);
	Log::Print("Written: " + TargetFolder + "/GENERATED_ObjectIncludes.h");
	for (unsigned int i = 0; i < Objects.size(); i++)
	{
		OutStream << "#include <Objects" + Objects[i].Path + "/" + Objects[i].Name + ".h>" << std::endl;
	}
	OutStream.close();
}

void WriteSpawnList(std::vector<Object> Objects, std::string TargetFolder)
{
	std::ofstream OutStream = std::ofstream(TargetFolder + "/GENERATED_Spawnlist.h", std::ios::out);
	Log::Print("Written: " + TargetFolder + "/GENERATED_Spawnlist.h");
	for (unsigned int i = 0; i < Objects.size(); i++)
	{
		uint32_t ID = hash_str_uint32(Objects[i].Name);
		OutStream << "case " + std::to_string(ID) + ": return (WorldObject*)SpawnObject<" + Objects[i].Name + ">(ObjectTransform); " << std::endl;
	}
	OutStream.close();
}

void WriteCategoryList(std::vector<Object> Objects, std::string TargetFolder)
{
	std::ofstream OutStream = std::ofstream(TargetFolder + "/GENERATED_Categories.h", std::ios::out);
	Log::Print("Written: " + TargetFolder + "/GENERATED_Categories.h");
	for (unsigned int i = 0; i < Objects.size(); i++)
	{
		uint32_t ID = hash_str_uint32(Objects[i].Name);
		OutStream << "case " + std::to_string(ID) + ": return " + Objects[i].Name + "::GetCategory();" << std::endl;
	}
	OutStream.close();
}

void WriteHeaderForObject(std::string TargetFolder, Object Object)
{
	Log::Print("Written: " + TargetFolder + "/GENERATED_" + Object.Name + ".h");
	unsigned int ID = hash_str_uint32(Object.Name);
	std::ofstream OutStream = std::ofstream(TargetFolder + "/GENERATED_" + Object.Name + ".h", std::ios::out);
	OutStream << "#pragma once" << std::endl;
	std::string UppercaseName = Object.Name;
	std::transform(Object.Name.begin(), Object.Name.end(), UppercaseName.begin(), ::toupper);
	OutStream << "#define " << UppercaseName << "_GENERATED(Category) "
		<< Object.Name << "() : WorldObject(ObjectDescription(\"" << Object.Name << "\", " << std::to_string(ID) << ")) {}\\\n"
		<< "static std::string GetCategory() { return Category; }\\\n";
	OutStream << "static uint32_t GetID() { return " << std::to_string(ID) << ";}";
	OutStream.close();
}

void RecursiveSearch(std::string InLoc, std::vector<Object>& Objects, std::string RelativePath = "")
{

	for (const auto& entry : std::filesystem::directory_iterator(InLoc))
	{
		if (entry.is_directory())
		{
			std::string DirName = entry.path().filename().string();
			if (DirName != "Components")
			{
				Log::Print(DirName);

				RecursiveSearch(entry.path().string(), Objects, RelativePath + "/" + DirName);
			}
		}
		else
		{
			std::string Filename = entry.path().string();
			auto Begin = std::max(Filename.find_last_of("/"), Filename.find_last_of("\\"));
			std::string Name = Filename.substr(Begin + 1, Filename.find_last_of(".") - Begin - 1);
			// Ignore Objects.h header
			if (Name != "Objects" && Name != "WorldObject")
			{
				auto Ext = Filename.substr(Filename.find_last_of(".") + 1);
				if (Ext == "h" || Ext == "hpp")
				{
					Objects.push_back(Object(Name, RelativePath));
				}
			}
		}
	}
}

int main(int argc, char** argv)
{
	try
	{
		std::vector<std::string> InLoc;
		std::string OutLoc;
		for (int i = 0; i < argc; i++)
		{
			std::string ArgStr = argv[i];
			if (ArgStr.substr(0, ArgStr.find_first_of("=")) == "In")
			{
				ArgStr = ArgStr.substr(ArgStr.find_first_of("=") + 1);
				if (!std::filesystem::exists(ArgStr))
				{
					throw ParseException("In path does not exist: " + ArgStr);
				}
				InLoc.push_back(ArgStr);
			}

			if (ArgStr.substr(0, ArgStr.find_first_of("=")) == "Out")
			{
				if (OutLoc.empty())
				{
					ArgStr = ArgStr.substr(ArgStr.find_first_of("=") + 1);
					OutLoc = ArgStr;
				}
				else
				{
					throw ParseException("Output path has been defined more than once");
				}
			}
		}

		if (InLoc.empty())
		{
			throw ParseException("In path has not been defined (In=[Path to Code/Objects folder])");
		}
		if (OutLoc.empty())
		{
			throw ParseException("Out path has not been defined (Out=[Path where to put the headers])");
		}

		std::filesystem::create_directories(OutLoc);
		std::vector<Object> Objects;
		for (const auto& location : InLoc)
		{
			RecursiveSearch(location, Objects);
		}

		WriteIncludeList	(Objects, OutLoc);
		WriteObjectList		(Objects, OutLoc);
		WriteSpawnList		(Objects, OutLoc);
		WriteCategoryList	(Objects, OutLoc);
		for (unsigned int i = 0; i < Objects.size(); i++)
		{
			WriteHeaderForObject(OutLoc, Objects[i]);
		}
	}
	catch (Exception& e)
	{
		Log::Print(e.What(), Log::E_ERROR);
	}
}