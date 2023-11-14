#include "ShaderPreprocessor.h"
#include <vector>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <Engine/Log.h>
#include <Engine/Build/Pack.h>
#include <map>
#include <cstring>
#include <Engine/Utility/StringUtility.h>

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

	std::map<std::string, Type::TypeEnum> GLSLTypes =
	{
		std::pair("int", Type::Int),
		std::pair("vec3", Type::Vector3),
		std::pair("float", Type::Float),
		std::pair("sampler2D", Type::GL_Texture),
		std::pair("bool", Type::Bool),
	};
}

Preprocessor::ProcessedShader Preprocessor::ParseGLSL(const std::string& Code, std::string Path)
{
	std::stringstream CodeStream;
	CodeStream << Code;
	size_t StringPos = 0;
	bool ReadingParameters = false;

	std::vector<Material::Param> Params;

	std::string Description;
	std::string Category;

	if (CodeStream.eof()) Log::Print(Code, Log::LogColor::Red);
	while (!CodeStream.eof())
	{
		char Line[4096];
		CodeStream.getline(Line, 4096);
		std::stringstream CurrentLine;
		std::string LineString = Line;
		StrUtil::ReplaceChar(LineString, '=', " = ");
		CurrentLine << LineString;
		StringPos += CurrentLine.str().size();
		std::string LineStart;
		CurrentLine >> LineStart;
		std::string NextWord;
		CurrentLine >> NextWord;
		// Dies the line start with a #params instruction?
		if (LineStart.substr(0, 2) == "//")
		{
			if (NextWord == "#params")
			{
				CurrentLine >> NextWord;
				Category = NextWord;

				if (Category == "#params")
				{
					Category.clear();
				}

				ReadingParameters = true;
				continue;
			}

			else if (ReadingParameters)
			{
				Description = CurrentLine.str().substr(2);
			}
		}
		else if (LineStart == "//#params")
		{
			Category = NextWord;
			ReadingParameters = true;
			continue;
		}
		else if (std::strspn(LineStart.c_str(), "	 ") == LineStart.size())
		{
			ReadingParameters = false;
		}
		else if (ReadingParameters)
		{
			if (LineStart != "uniform")
			{
				Log::Print("--------------------------------------------------------------------------------", Log::LogColor::Red);
				Log::Print("ShaderParseError: expected a uniform in parameter list.", Log::LogColor::Red);
				Log::Print(Line + std::string(" <--"), Log::LogColor::Red);
				throw "ShaderParseError";

			}
			if (NextWord.empty() || !GLSLTypes.contains(NextWord))
			{
				Log::Print("--------------------------------------------------------------------------------", Log::LogColor::Red);
				Log::Print("ShaderParseError: expected a valid type.", Log::LogColor::Red);
				Log::Print(Line + std::string(" <--"), Log::LogColor::Red);
				Log::Print("Possible Types: ", Log::LogColor::Red);
				for (auto& i : GLSLTypes)
				{
					Log::Print("    " + i.first, Log::LogColor::Red);
				}
				throw "ShaderParseError";
			}
			Type::TypeEnum ParamType = GLSLTypes[NextWord];
			//Params.push_back(Material::Param(CurrentLine.str(), ));
			std::string Name;
			CurrentLine >> Name;
			StrUtil::ReplaceChar(Name, ';', "");

			std::string Default;
			CurrentLine >> Default;
			if (Default == "=")
			{
				switch (ParamType)
				{
				case Type::Vector3:
				{
					// xy = vec3(1, 2, 3);
					LineString = LineString.substr(LineString.find_first_of("=") + 1);
					// vec3(1, 2, 3);
					StrUtil::ReplaceChar(LineString, '(', " ");
					StrUtil::ReplaceChar(LineString, ')', " ");
					StrUtil::ReplaceChar(LineString, ',', " ");
					StrUtil::ReplaceChar(LineString, ';', "");
					// vec3 1 2 3


					std::stringstream VecStream;
					VecStream << LineString;
					std::string OutString;
					VecStream >> OutString;
					// VecStream: 1, 2, 3, OutString = vec3
					if (OutString != "vec3")
					{
						Default.clear();
						break;
					}

					OutString.clear();

					std::string VecString;
					size_t Values = 0;

					while (!VecStream.eof())
					{
						VecString.append(OutString + " ");
						VecStream >> OutString;
						Values++;
					}
					if (Values == 2)
					{
						VecString = VecString + VecString + VecString;
					}
					Default = VecString;

				}
					break;
				case Type::Float:
				case Type::Int:
				{
					CurrentLine >> Default;
					StrUtil::ReplaceChar(Default, ';', "");
				}
					break;
				case Type::GL_Texture:
					Default.clear();
					break;
				case Type::Bool:
				{
					CurrentLine >> Default;
					StrUtil::ReplaceChar(Default, ';', "");
					if (Default == "true")
					{
						Default = "1";
					}
					else if (Default == "false")
					{
						Default = "0";
					}
					else
					{
						Log::Print(Default);
						Default.clear();
					}
				}
					break;
				default:
					break;
				}
			}
			else
			{
				Default.clear();
			}

			auto DescFirstNonWhitespace = Description.find_first_not_of(" ");
			if (DescFirstNonWhitespace != std::string::npos)
			{
				Description = Description.substr(DescFirstNonWhitespace);
			}
			Params.push_back(Material::Param(Name, ParamType, Default, Category + "#" + Description));
			Description.clear();
		}

		// Does the line start with a preprocessor include instruction?
		if (LineStart.substr(0, 11) != "//!#include")
		{
			if (LineStart == "//!")
			{
				if (NextWord.substr(0, 8) != "#include")
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

		const std::string FileToParse = RemoveQuotesFromString(Instruction);
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
		auto ParseResult = ParseGLSL(IncludeStream.str(), Path);
#else
		auto ParseResult = ParseGLSL(Pack::GetFile(FileToParse), Path);
#endif
		ReadingParameters = false;
		Params.clear();

		const std::string& IncludeString = ParseResult.Code;


		std::string NewCode = CodeStream.str().substr(0, StringPos - CurrentLine.str().size());
		NewCode.append(IncludeString);
		NewCode.append(CodeStream.str().substr(StringPos));
		CodeStream = std::stringstream();
		CodeStream << NewCode;
	}

	ProcessedShader ReturnValue;
	ReturnValue.Code = CodeStream.str();
	ReturnValue.ShaderParams = Params;
	return ReturnValue;
}
