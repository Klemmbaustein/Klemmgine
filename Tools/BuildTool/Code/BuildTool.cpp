#include "Log.h"
#include <vector>
#include <filesystem>
#include <fstream>
#include <algorithm>

void ParseError(std::string Reason, std::string File, std::string Function, size_t Line)
{
	Log::Print(Reason, Log::MessageType::Error, File);
	exit(1);
}

#define PARSE_ERROR(reason) ParseError(reason, __FILE__, __FUNCTION__, __LINE__)

// 32bit string hash for generating the ID of an object
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

void WriteToFile(std::string str, std::string File)
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

void WriteObjectList(std::vector<Object> Objects, std::string TargetFolder)
{
	std::string OutStream;
	for (unsigned int i = 0; i < Objects.size(); i++)
	{
		uint32_t ID = hash_str_uint32(Objects[i].Name);
		OutStream.append("ObjectDescription(\"" + Objects[i].Name + "\", " + std::to_string(ID) + "),\n");
	}
	WriteToFile(OutStream, TargetFolder + "/GENERATED_ListOfObjects.h");
}

void WriteIncludeList(std::vector<Object> Objects, std::string TargetFolder)
{
	std::stringstream OutStream;
	for (unsigned int i = 0; i < Objects.size(); i++)
	{
		OutStream << "#include <Objects" + Objects[i].Path + "/" + Objects[i].Name + ".h>" << std::endl;
	}
	WriteToFile(OutStream.str(), TargetFolder + "/GENERATED_ObjectIncludes.h");
}

void WriteSpawnList(std::vector<Object> Objects, std::string TargetFolder)
{
	std::stringstream OutStream;
	for (unsigned int i = 0; i < Objects.size(); i++)
	{
		uint32_t ID = hash_str_uint32(Objects[i].Name);
		OutStream << "case " + std::to_string(ID) + ": return (WorldObject*)SpawnObject<" + Objects[i].Name + ">(ObjectTransform, NetID); " << std::endl;
	}
	WriteToFile(OutStream.str(), TargetFolder + "/GENERATED_Spawnlist.h");
}

void WriteCategoryList(std::vector<Object> Objects, std::string TargetFolder)
{
	std::stringstream OutStream;
	for (unsigned int i = 0; i < Objects.size(); i++)
	{
		uint32_t ID = hash_str_uint32(Objects[i].Name);
		OutStream << "case " + std::to_string(ID) + ": return " + Objects[i].Name + "::GetCategory();" << std::endl;
	}
	WriteToFile(OutStream.str(), TargetFolder + "/GENERATED_Categories.h");
}

void WriteHeaderForObject(std::string TargetFolder, Object Object)
{
	unsigned int ID = hash_str_uint32(Object.Name);
	std::stringstream OutStream;
	std::string UppercaseName = Object.Name;
	std::transform(Object.Name.begin(), Object.Name.end(), UppercaseName.begin(), ::toupper);
	OutStream << "#define " << UppercaseName << "_GENERATED(Category) "
		<< Object.Name << "() : WorldObject(ObjectDescription(\"" << Object.Name << "\", " << std::to_string(ID) << ")) {}\\\n"
		<< "static std::string GetCategory() { return Category; }\\\n";
	OutStream << "static uint32_t GetID() { return " << std::to_string(ID) << ";}";
	WriteToFile(OutStream.str(), TargetFolder + "/" + Object.Name + ".h");
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
				RecursiveSearch(entry.path().string(), Objects, RelativePath + "/" + DirName);
			}
		}
		else
		{
			std::string Filename = entry.path().string();
			auto Begin = Filename.find_last_of("/\\");
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
	std::vector<std::string> InLoc;
	std::string OutLoc;
	for (int i = 0; i < argc; i++)
	{
		std::string ArgStr = argv[i];
		if (ArgStr.substr(0, ArgStr.find_first_of("=")) == "in")
		{
			ArgStr = ArgStr.substr(ArgStr.find_first_of("=") + 1);
			if (!std::filesystem::exists(ArgStr))
			{
				PARSE_ERROR("In path does not exist: " + ArgStr + " - Current Path: " + std::filesystem::current_path().u8string());
			}
			InLoc.push_back(ArgStr);
		}

		if (ArgStr.substr(0, ArgStr.find_first_of("=")) == "out")
		{
			if (OutLoc.empty())
			{
				ArgStr = ArgStr.substr(ArgStr.find_first_of("=") + 1);
				OutLoc = ArgStr;
			}
			else
			{
				PARSE_ERROR("Output path has been defined more than once");
			}
		}
	}

	if (InLoc.empty())
	{
		PARSE_ERROR("In path has not been defined (In=[Path to Code/Objects folder])");
	}
	if (OutLoc.empty())
	{
		PARSE_ERROR("Out path has not been defined (Out=[Path where to put the headers])");
	}

	OutLoc.append("/GENERATED");
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