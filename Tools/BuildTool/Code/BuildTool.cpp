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

void WriteObjectList(std::vector<std::string> Objects, std::string TargetFolder)
{
	std::ofstream OutStream = std::ofstream(TargetFolder + "/GENERATED_ListOfObjects.h", std::ios::out);
	Log::Print("Written: " + TargetFolder + "/GENERATED_ListOfObjects.h");
	for (unsigned int i = 0; i < Objects.size(); i++)
	{
		uint32_t ID = hash_str_uint32(Objects[i]);
		OutStream << "ObjectDescription(\"" << Objects[i] << "\", " << ID << ")," << std::endl;
	}
	OutStream.close();
}

void WriteIncludeList(std::vector<std::string> Objects, std::string TargetFolder)
{
	std::ofstream OutStream = std::ofstream(TargetFolder + "/GENERATED_ObjectIncludes.h", std::ios::out);
	Log::Print("Written: " + TargetFolder + "/GENERATED_ObjectIncludes.h");
	for (unsigned int i = 0; i < Objects.size(); i++)
	{
		OutStream << "#include <Objects/" + Objects[i] + ".h>" << std::endl;
	}
	OutStream.close();
}

void WriteSpawnList(std::vector<std::string> Objects, std::string TargetFolder)
{
	std::ofstream OutStream = std::ofstream(TargetFolder + "/GENERATED_Spawnlist.h", std::ios::out);
	Log::Print("Written: " + TargetFolder + "/GENERATED_Spawnlist.h");
	for (unsigned int i = 0; i < Objects.size(); i++)
	{
		uint32_t ID = hash_str_uint32(Objects[i]);
		OutStream << "case " + std::to_string(ID) + ": return (WorldObject*)SpawnObject<" + Objects[i] + ">(ObjectTransform); " << std::endl;
	}
	OutStream.close();
}

void WriteCategoryList(std::vector<std::string> Objects, std::string TargetFolder)
{
	std::ofstream OutStream = std::ofstream(TargetFolder + "/GENERATED_Categories.h", std::ios::out);
	Log::Print("Written: " + TargetFolder + "/GENERATED_Categories.h");
	for (unsigned int i = 0; i < Objects.size(); i++)
	{
		uint32_t ID = hash_str_uint32(Objects[i]);
		OutStream << "case " + std::to_string(ID) + ": return " + Objects[i] + "::GetCategory();" << std::endl;
	}
	OutStream.close();
}

void WriteHeaderForObject(std::string TargetFolder, std::string Object)
{
	Log::Print("Written: " + TargetFolder + "/GENERATED_" + Object + ".h");
	unsigned int ID = hash_str_uint32(Object);
	std::ofstream OutStream = std::ofstream(TargetFolder + "/GENERATED_" + Object + ".h", std::ios::out);
	OutStream << "#pragma once" << std::endl;
	std::string UppercaseName = Object;
	std::transform(Object.begin(), Object.end(), UppercaseName.begin(), ::toupper);
	OutStream << "#define " << UppercaseName << "_GENERATED(Category) "
		<< Object << "() : WorldObject(ObjectDescription(\"" << Object << "\", " << std::to_string(ID) << ")) {}\\\n"
		<< "static std::string GetCategory() { return Category; }\\\n";
	OutStream << "static uint32_t GetID() { return " << std::to_string(ID) << ";}";
	OutStream.close();
}

void RecursiveSearch(std::string InLoc, std::vector<std::string>& Objects)
{

	for (const auto& entry : std::filesystem::directory_iterator(InLoc))
	{
		if (entry.is_directory())
		{
			RecursiveSearch(entry.path().string(), Objects);
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
					Objects.push_back(Name);
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
		std::vector<std::string> Objects;
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