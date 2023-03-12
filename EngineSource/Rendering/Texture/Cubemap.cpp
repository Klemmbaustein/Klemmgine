#include "Cubemap.h"
#include <Engine/Save.h>
#include <Engine/Console.h>
#include <Engine/Log.h>
#include <World/Assets.h>
#include <filesystem>
#include <Rendering/Texture/Texture.h>
#include <GL/glew.h>

unsigned int Cubemap::LoadCubemapFile(std::string File)
{
	std::string FileAsset = Assets::GetAsset(File + ".cbm");
	if (std::filesystem::exists(FileAsset))
	{
		SaveGame SaveFile = SaveGame(FileAsset.substr(0, FileAsset.size() - 4), "cbm", false);
		std::vector<std::string> CubemapFiles = {"", "", "", "", "", ""};
		std::vector<std::string> Cubenames = {"right", "left", "down", "up", "front", "back"};

		for (size_t i = 0; i < Cubenames.size(); i++)
		{
			CubemapFiles[i] = SaveFile.GetPropterty(Cubenames[i]).Value;
		}
		return Texture::LoadCubemapTexture(CubemapFiles);
	}
	else
	{
		Log::Print("Could not find " + File + ".cbm");
		return 0;
	}
}

void Cubemap::UnloadCubemapFile(unsigned int map)
{
	glDeleteTextures(1, &map);
}

void Cubemap::RegisterCommands()
{
	Console::RegisterCommand(Console::Command("cubetest", []() { LoadCubemapFile(Console::CommandArgs()[0]); }, { Console::Command::Argument("cubemap", Type::E_STRING)}));
}

