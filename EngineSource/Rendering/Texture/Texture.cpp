#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <Utility/stb_image.hpp>
#include <GL/glew.h>
#include <Engine/Log.h>
#include <filesystem>
#include <Engine/Application.h>


namespace Assets
{
	std::string GetAsset(std::string File);
}

namespace Texture
{
	std::vector<Texture> Textures;

	unsigned int LoadTexture(std::string File)
	{
		for (Texture& t : Textures)
		{
			if (t.TexturePath == File)
			{
				++t.References;
				return t.TextureID;
			}
		}
		try
		{
			std::string TextureFile = File;
			if (!std::filesystem::exists(TextureFile))
			{
				TextureFile = Assets::GetAsset(File + ".png");
			}

			if (!std::filesystem::exists(TextureFile))
			{
				return 0;
			}

			int TextureWidth = 0;
			int TextureHeigth = 0;
			int BitsPerPixel = 0;
			stbi_set_flip_vertically_on_load(true);
			auto TextureBuffer = stbi_load(TextureFile.c_str(), &TextureWidth, &TextureHeigth, &BitsPerPixel, 4);
			GLuint TextureID;
			glGenTextures(1, &TextureID);
			glBindTexture(GL_TEXTURE_2D, TextureID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TextureWidth, TextureHeigth, 0, GL_RGBA, GL_UNSIGNED_BYTE, TextureBuffer);

			Textures.push_back(Texture(TextureID, 1, File));
			if (TextureBuffer)
			{
				stbi_image_free(TextureBuffer);
			}
			return TextureID;
		}
		catch (std::exception& e)
		{
			Log::Print(std::string("Error loading Texture: ") + e.what(), Vector3(0.7f, 0.f, 0.f));
			return 0;
		}
	}

	unsigned int CreateTexture(TextureData T)
	{
		unsigned int TextureID;
		glGenTextures(1, &TextureID);
		glBindTexture(GL_TEXTURE_2D, TextureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, T.ResolutionX, T.ResolutionY, 0, GL_RGBA, GL_FLOAT, (void*)T.Pixels.data());

		return TextureID;
	}

	unsigned int LoadCubemapTexture(std::vector<std::string> Files)
	{
		std::vector<bool> IsJPEG = { false, false, false, false, false, false };
		for (int i = 0; i < Files.size(); i++)
		{
			if (std::filesystem::exists(Files[i]))
			{
				continue;
			}
			std::string NewFile = Assets::GetAsset(Files[i] + ".png");
			if (std::filesystem::exists(NewFile))
			{
				Files[i] = NewFile;
				continue;
			}
			NewFile = Assets::GetAsset(Files[i] + ".jpg");
			if (std::filesystem::exists(NewFile))
			{
				IsJPEG[i] = true;
				Files[i] = NewFile;
				continue;
			}
			//Log::Print("Error: Couldn't find " + Files[i], Log::LogColor::Red);
		}
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		int width = 0, height = 0, nrChannels;
		for (unsigned int i = 0; i < Files.size(); i++)
		{
			int newWidth, newHeight;
			unsigned char* data = stbi_load(Files[i].c_str(), &newWidth, &newHeight, &nrChannels, 0);
			if (width == 0)
			{
				width = newWidth;
				height = newHeight;
			}
			else if (width != newWidth || height != newHeight)
			{
				Log::Print("Cubemap loading error: Textures don't match in size. (Previous: " + std::to_string(width) + "x" + std::to_string(height)
					+ ", new: " + std::to_string(newWidth) + "x" + std::to_string(newHeight) + ")", Log::LogColor::Red);
				stbi_image_free(data);
				return 0;
			}
			if (data)
			{
				int format = nrChannels == 3 ? GL_RGB : GL_RGBA;
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
				);
				stbi_image_free(data);
			}
			else
			{
				stbi_image_free(data);
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		return textureID;
	}

	void UnloadTexture(unsigned int TextureID)
	{
		for (int i = 0; i < Textures.size(); i++)
		{
			Texture& t = Textures.at(i);
			if (t.TextureID == TextureID)
			{
				--t.References;
				if (t.References <= 0)
				{
					glDeleteTextures(1, &Textures.at(i).TextureID);
					Textures.erase(Textures.begin() + i);
				}
				return;
			}
		}
		glDeleteTextures(1, &TextureID);
	}
}