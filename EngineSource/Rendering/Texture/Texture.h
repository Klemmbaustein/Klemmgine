#pragma once
#include <vector>
#include <string>

namespace Texture
{
	struct Texture
	{
		unsigned int TextureID = 0;
		unsigned int References = 0;
		std::string TexturePath;

		Texture(unsigned int TextureID,
		unsigned int References ,
		std::string TexturePath)
		{
			this->TextureID = TextureID;
			this->References = References;
			this->TexturePath = TexturePath;
		}
	};

	extern std::vector<Texture> Textures;

	unsigned int LoadTexture(std::string File);

	unsigned int LoadCubemapTexture(std::vector<std::string> Files);

	void UnloadTexture(unsigned int TextureID);
}