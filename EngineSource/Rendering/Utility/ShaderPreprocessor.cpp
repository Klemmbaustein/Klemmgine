#include "ShaderPreprocessor.h"
#include <vector>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <Engine/Log.h>
#include <Engine/Build/Pack.h>

namespace Preprocessor
{
	std::string RemoveQuotesFromString(std::string InString)
	{
		if (InString[0] == '"' && InString[InString.size() - 1] == '"')
		{
			return InString.substr(1, InString.size() - 2);
		}
		if (InString[0] == '<' && InString[InString.size() - 1] == '>')
		{
			return InString.substr(1, InString.size() - 2);
		}
		return "\n";
	}
}

std::string Preprocessor::ParseGLSL(std::string Code, std::string Path)
{
	std::stringstream CodeStream;
	CodeStream << Code;
	unsigned int LineInt = 0;
	size_t StringPos = 0;
	if (CodeStream.eof()) Log::Print(Code, Log::LogColor::Red);
	while (!CodeStream.eof())
	{
		LineInt++;
		char Line[256];
		CodeStream.getline(Line, 256);
		std::stringstream CurrentLine;
		CurrentLine << Line;
		StringPos += CurrentLine.str().size();
		std::string LineStart;
		CurrentLine >> LineStart;

		// Does the line start with a preprocessor instruction?
		if (LineStart.substr(0, 11) != "//!#include")
		{
			if (LineStart == "//!")
			{
				CurrentLine >> LineStart;
				if (LineStart.substr(0, 8) != "#include")
				{
					continue;
				}
			}
			else continue;
		}

		std::string Instruction;
		CurrentLine >> Instruction;
#if !RELEASE
		if (!CurrentLine.eof())
		{
			Log::Print("--------------------------------------------------------------------------------", Log::LogColor::Red);
			Log::Print("ShaderParseError: Unexpected arguments after '" + Instruction + "'", Log::LogColor::Red);
			Log::Print(Line + std::string(" <--"), Log::LogColor::Red);
			while (!CurrentLine.eof())
			{
				std::string str;
				CurrentLine >> str;
				Log::Print(str);
			}
			throw "ShaderParseError";
		}
#endif

		std::string FileToParse = RemoveQuotesFromString(Instruction);
		if (FileToParse == "\n")
		{
			Log::Print("--------------------------------------------------------------------------------", Log::LogColor::Red);
			Log::Print("ShaderParseError: Expected quotes around string " + Instruction, Log::LogColor::Red);
			Log::Print(Line + std::string(" <--"), Log::LogColor::Red);
			throw "ShaderParseError";
		}
#if !RELEASE
		if (!std::filesystem::exists(Path + "/" + FileToParse))
		{
			Log::Print("--------------------------------------------------------------------------------", Log::LogColor::Red);
			Log::Print("ShaderParseError: File does not exist: " + FileToParse, Log::LogColor::Red);
			Log::Print("Include path is: " + Path, Log::LogColor::Red);
			Log::Print(Line + std::string(" <--"), Log::LogColor::Red);
			throw "ShaderParseError";
		}

		std::ifstream IncludeFile;
		IncludeFile.open(Path + "/" + FileToParse);
		std::stringstream IncludeStream;
		// read file's buffer contents into streams
		IncludeStream << IncludeFile.rdbuf();
		// close file handlers
		IncludeFile.close();
		// convert stream into string
		std::string IncludeString = ParseGLSL(IncludeStream.str(), Path);
#else
		std::string IncludeString = ParseGLSL(Pack::GetFile(FileToParse), Path);
#endif

		std::string NewCode = CodeStream.str().substr(0, StringPos - CurrentLine.str().size());
		NewCode.append(IncludeString);
		NewCode.append(CodeStream.str().substr(StringPos));
		CodeStream = std::stringstream();
		CodeStream << NewCode;
	}
	return CodeStream.str();
}
