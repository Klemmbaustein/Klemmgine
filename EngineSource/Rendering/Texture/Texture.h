#pragma once
#include <Math/Vector.h>

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

	struct TextureData
	{
		std::vector<Vector3> Pixels;
		unsigned int ResolutionX = 0, ResolutionY = 0;
	};

	enum class TextureFiltering
	{
		Nearest,
		Linear
	};

	enum class TextureWrap
	{
		Clamp,
		Border,
		Repeat
	};

	extern std::vector<Texture> Textures;

	struct TextureInfo
	{
		std::string File;
		TextureFiltering Filtering = TextureFiltering::Nearest;
		TextureWrap Wrap = TextureWrap::Clamp;
	};
	
	TextureInfo ParseTextureInfoString(std::string TextureInfoString);
	std::string CreateTextureInfoString(TextureInfo TextureInfo);

	unsigned int LoadTexture(std::string File, TextureFiltering Filtering = TextureFiltering::Nearest, TextureWrap Wrap = TextureWrap::Clamp);
	unsigned int LoadTexture(TextureInfo T);
	unsigned int CreateTexture(TextureData T);

	unsigned int LoadCubemapTexture(std::vector<std::string> Files);

	void UnloadTexture(unsigned int TextureID);
}