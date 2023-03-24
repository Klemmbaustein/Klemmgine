#include "Pack.h"
#include <filesystem>
#include <Engine/Log.h>
#include <fstream>
#include <Engine/FileUtility.h>

std::vector<Pack::PackFile> Pack::GetPackContents(std::string Pack)
{
	std::vector<PackFile> LoadedFiles;
	if (std::filesystem::exists(Pack))
	{
		std::ifstream In = std::ifstream(Pack, std::ios::in | std::ios::binary);
		size_t AmmountOfFiles;
		In.read((char*)&AmmountOfFiles, sizeof(size_t));

		for(uint32_t i = 0; i < AmmountOfFiles; i++)
		{
			std::string inName;
			size_t stringlen;
			In.read((char*)&stringlen, sizeof(size_t));
			char* temp = new char[stringlen + 1];
			In.read(temp, stringlen);
			temp[stringlen] = '\0';
			inName = temp;
			delete[] temp;

			std::string inContent;
			stringlen;
			In.read((char*)&stringlen, sizeof(size_t));
			temp = new char[stringlen + 1];
			In.read(temp, stringlen);
			temp[stringlen] = '\0';
			inContent = temp;
			delete[] temp;
			LoadedFiles.push_back(PackFile(inName, inContent));
		}
		In.close();
		return LoadedFiles;
	}
	Log::Print("Error: Tried to load Pack " + Pack + " but it does not exist", Vector3(1, 0, 0));
	return LoadedFiles;
}

bool Pack::SaveToPack(std::vector<PackFile> Content, std::string Path)
{
	std::ofstream Out = std::ofstream(Path, std::ios::binary | std::ios::out);
	size_t AmmountOfFiles = Content.size();
	Out.write((char*)&AmmountOfFiles, sizeof(size_t));
	for (PackFile& p : Content)
	{
		size_t FileNameSize = p.FileName.size();

		Out.write((char*)&FileNameSize, sizeof(size_t));
		Out.write(p.FileName.c_str(), sizeof(char) * p.FileName.size());

		size_t ContentSize = p.Content.size();

		Out.write((char*)&ContentSize, sizeof(size_t));
		Out.write(p.Content.c_str(), sizeof(char) * p.Content.size());
	}
	Out.close();
	return true;
}

void Pack::SaveFolderToPack(std::string Folder, std::string Outf)
{
	std::vector<PackFile> PackedFiles;
	if (std::filesystem::is_directory(Folder))
	{
		for (const auto& entry : std::filesystem::directory_iterator(Folder))
		{
			if (!entry.is_directory())
			{
				std::ifstream InFile = std::ifstream(entry.path(), std::ios::in | std::ios::binary);
				std::stringstream InStream;
				InStream << InFile.rdbuf();
				PackedFiles.push_back(PackFile(FileUtil::GetFileNameFromPath(entry.path().string()), InStream.str()));
				InFile.close();
			}
		}
	}
	else
	{
		throw "Pack dir invalid";
	}
	SaveToPack(PackedFiles, Outf);
}

std::string Pack::GetFile(std::string File)
{
	File = FileUtil::GetFileNameFromPath(File);
	for (const auto& entry : std::filesystem::directory_iterator("Assets/"))
	{
		if (entry.path().string().substr(entry.path().string().find_last_of('.') + 1) == "pack")
		{
			auto CurrentPack = GetPackContents(entry.path().string());
			for (auto& p : CurrentPack)
			{
				if (p.FileName == File)
				{
					return p.Content;
				}
			}
		}
	}
	throw "Could not find file";
}