#pragma once
#include <Math/Vector.h>

/**
* @file
*/

/**
* @brief
* Texture namespace
*/
namespace Texture
{
	/// An OpenGL texture. Loaded with LoadTexture() or CreateTexture(), unloaded with UnloadTexture()
	typedef unsigned int TextureType;
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

	/// Contains data for creating a new texture.
	struct TextureData
	{
		/// The pixels of that texture.
		std::vector<Vector3> Pixels;
		/// The width of the texture in pixels.
		unsigned int ResolutionX = 0;
		/// The height of the texture in pixels.
		unsigned int ResolutionY = 0;
	};

	/// Filtering used to control how the texture is interpolated.
	enum class TextureFiltering
	{
		/// Nearest filtering. Pixelated look.
		Nearest,
		/// Linear filtering. Blurry look.
		Linear
	};

	/// Texture wrap mode used to control how the texture's edges are handled.
	enum class TextureWrap
	{
		/// The texture coordinates are clamped, so the edge pixels are stretched around the border.
		Clamp,
		/// A black border is put around the texture.
		Border,
		/// The texture repeats.
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


	/**
	* @brief
	* Loads a texture
	* 
	* @param File
	* The texture file.
	* 
	* @param Filtering
	* The filtering of the texture.
	* 
	* @param Wrap
	* The wrap mode of the texture.
	* 
	* @return
	* The created texture or TextureType(0) if it failed.
	*/
	TextureType LoadTexture(std::string File, TextureFiltering Filtering = TextureFiltering::Nearest, TextureWrap Wrap = TextureWrap::Clamp);
	TextureType LoadTexture(TextureInfo T);

	/**
	* @brief
	* Creates a texture from the given data.
	* 
	* @param T
	* Pixel data of the texture.
	* 
	* @param Filtering
	* The filtering of the texture.
	*
	* @param Wrap
	* The wrap mode of the texture.
	* 
	* @return
	* The created texture or TextureType(0) if it failed.
	*/
	TextureType CreateTexture(TextureData T, TextureFiltering Filtering = TextureFiltering::Nearest, TextureWrap Wrap = TextureWrap::Clamp);

	TextureType LoadCubemapTexture(std::vector<std::string> Files);

	/**
	* @brief
	* Unloads the given texture.
	*/
	void UnloadTexture(TextureType TextureID);
}