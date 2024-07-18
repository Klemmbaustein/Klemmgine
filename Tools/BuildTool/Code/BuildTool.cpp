#include "Log.h"
#include <vector>
#include <filesystem>
#include "ParseFile.h"

void ParseError(std::string Reason, std::string File, size_t Line)
{
	std::string Name = File.substr(File.find_last_of("/\\") + 1);

	Log::Print(Reason, Log::MessageType::Error, Name);
	exit(1);
}

#define PARSE_ERROR(reason) ParseError(reason, __FILE__, __LINE__)

void WriteObjectList(std::vector<ParseFile::Object> Objects, std::string TargetFolder)
{
	std::string OutStream;
	for (unsigned int i = 0; i < Objects.size(); i++)
	{
		OutStream.append("ObjectDescription(\"" + Objects[i].Name + "\", " + std::to_string(Objects[i].Hash) + "),\n");
	}
	ParseFile::WriteToFile(OutStream, TargetFolder + "/GENERATED_ListOfObjects.h");
}

void WriteIncludeList(std::vector<ParseFile::Object> Objects, std::string TargetFolder)
{
	std::stringstream OutStream;
	for (unsigned int i = 0; i < Objects.size(); i++)
	{
		std::string Path = Objects[i].Path;

		if (Path.substr(0, 2) == "./")
		{
			Path = Path.substr(2);
		}

		OutStream << "#include <" << Path << ">" << std::endl;
	}
	ParseFile::WriteToFile(OutStream.str(), TargetFolder + "/GENERATED_ObjectIncludes.h");
}

void WriteSpawnList(std::vector<ParseFile::Object> Objects, std::string TargetFolder)
{
	std::stringstream OutStream;
	for (unsigned int i = 0; i < Objects.size(); i++)
	{
		OutStream << "case " + std::to_string(Objects[i].Hash) + ": return (SceneObject*)SpawnObject<" + Objects[i].Name + ">(ObjectTransform, NetID); " << std::endl;
	}
	ParseFile::WriteToFile(OutStream.str(), TargetFolder + "/GENERATED_Spawnlist.h");
}

void WriteCategoryList(std::vector<ParseFile::Object> Objects, std::string TargetFolder)
{
	std::stringstream OutStream;
	for (unsigned int i = 0; i < Objects.size(); i++)
	{
		OutStream << "case " + std::to_string(Objects[i].Hash) + ": return " + Objects[i].Name + "::GetCategory();" << std::endl;
	}
	ParseFile::WriteToFile(OutStream.str(), TargetFolder + "/GENERATED_Categories.h");
}

void RecursiveSearch(std::string InLoc, std::vector<ParseFile::Object>& Objects, std::string RelativePath = "")
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
			std::vector<ParseFile::Object> NewObjects = ParseFile::ParseFile(std::filesystem::canonical(entry).string());
			for (auto& i : NewObjects)
			{
				Objects.push_back(i);
			}
		}
	}
}

int main(int argc, char** argv)
{
	std::vector<std::string> InLoc;
	std::string OutLoc;

	bool WriteCSharp = true;

	for (int i = 0; i < argc; i++)
	{
		std::string ArgStr = argv[i];
		if (ArgStr.substr(0, ArgStr.find_first_of("=")) == "in")
		{
			ArgStr = ArgStr.substr(ArgStr.find_first_of("=") + 1);
			if (!std::filesystem::exists(ArgStr))
			{
				PARSE_ERROR("In path does not exist: " + ArgStr + " - Current Path: " + std::filesystem::current_path().string());
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

		if (ArgStr == "noCSharp")
		{
			WriteCSharp = false;
		}
	}

	if (InLoc.empty())
	{
		PARSE_ERROR("In path has not been defined (in=[Path to Code/Objects folder])");
	}
	if (OutLoc.empty())
	{
		PARSE_ERROR("Out path has not been defined (out=[Path where to put the headers])");
	}

	OutLoc.append("/GENERATED");
	std::filesystem::create_directories(OutLoc);
	std::vector<ParseFile::Object> Objects;
	for (const auto& location : InLoc)
	{
		RecursiveSearch(location, Objects);
	}

	std::vector<ParseFile::Object> SceneObjects;
	for (ParseFile::Object& i : Objects)
	{
		if (i.Name == "CSharpObject" && !WriteCSharp)
		{
			continue;
		}
		if (i.DerivesFromSceneObject(Objects))
		{
			i.WriteGeneratedHeader(OutLoc);
			SceneObjects.push_back(i);
		}
	}
	WriteIncludeList(SceneObjects, OutLoc);
	WriteObjectList(SceneObjects, OutLoc);
	WriteSpawnList(SceneObjects, OutLoc);
	WriteCategoryList(SceneObjects, OutLoc);
}