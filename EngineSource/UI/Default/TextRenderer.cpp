#if !SERVER
#define _CRT_SECURE_NO_WARNINGS
#include "TextRenderer.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include <Utility/stb_truetype.hpp>
#include <Engine/Utility/FileUtility.h>
#include <UI/Default/ScrollObject.h>
#include <Engine/Log.h>
#include <Rendering/Shader.h>
#include <SDL.h>
#include <Engine/Stats.h>
#include <Rendering/Graphics.h>
#include <GL/glew.h>
#include <Engine/Utility/StringUtility.h>
#include <Engine/EngineError.h>
#include <Engine/OS.h>

// TODO: Correct handling of tabs like in the standalone ui library

static std::wstring GetUnicodeString(const std::string& utf8)
{
	std::vector<unsigned long> unicode;
	unicode.reserve(utf8.size());
	size_t i = 0;
	while (i < utf8.size())
	{
		unsigned long uni;
		size_t todo;
		bool error = false;
		unsigned char ch = utf8[i++];
		if (ch <= 0x7F)
		{
			uni = ch;
			todo = 0;
		}
		else if (ch <= 0xBF)
		{
			return std::wstring(utf8.begin(), utf8.end());
		}
		else if (ch <= 0xDF)
		{
			uni = ch & 0x1F;
			todo = 1;
		}
		else if (ch <= 0xEF)
		{
			uni = ch & 0x0F;
			todo = 2;
		}
		else if (ch <= 0xF7)
		{
			uni = ch & 0x07;
			todo = 3;
		}
		else
		{
			return std::wstring(utf8.begin(), utf8.end());
		}
		for (size_t j = 0; j < todo; ++j)
		{
			if (i == utf8.size())
				return std::wstring(utf8.begin(), utf8.end());
			unsigned char ch = utf8[i++];
			if (ch < 0x80 || ch > 0xBF)
				return std::wstring(utf8.begin(), utf8.end());
			uni <<= 6;
			uni += ch & 0x3F;
		}
		if (uni >= 0xD800 && uni <= 0xDFFF)
			return std::wstring(utf8.begin(), utf8.end());
		if (uni > 0x10FFFF)
			return std::wstring(utf8.begin(), utf8.end());
		unicode.push_back(uni);
	}
	std::wstring utf16;
	utf16.reserve(unicode.size());
	for (size_t i = 0; i < unicode.size(); ++i)
	{
		unsigned long uni = unicode[i];
		if (uni <= 0xFFFF)
		{
			utf16 += (wchar_t)uni;
		}
		else
		{
			uni -= 0x10000;
			utf16 += (wchar_t)((uni >> 10) + 0xD800);
			utf16 += (wchar_t)((uni & 0x3FF) + 0xDC00);
		}
	}
	return utf16;
}

constexpr int FONT_BITMAP_WIDTH = 2700;
constexpr int FONT_BITMAP_PADDING = 32;
constexpr int FONT_MAX_UNICODE_CHARS = 700;
constexpr int TAB_SIZE = 4;

size_t TextRenderer::GetCharacterIndexADistance(ColoredText Text, float Dist, float Scale)
{
	Scale *= 10.0f;
	std::wstring TextString = GetUnicodeString(TextSegment::CombineToString(Text));
	TextString.append(L" ");
	float MaxHeight = 0.0f;
	float x = 0.f, y = 0.f;
	size_t i = 0;
	size_t CharIndex = 0;
	float PrevDepth = 0;
	float PrevMaxDepth = 0;
	for (auto& c : TextString)
	{
		bool IsTab = false;
		if (c == L'\t')
		{
			c = L' ';
			IsTab = true;
		}
		if (c >= 32)
		{
			int GlyphIndex = (int)c - 32;
			if (GlyphIndex < 0)
			{
				continue;
			}
			if (GlyphIndex > FONT_MAX_UNICODE_CHARS)
			{
				GlyphIndex = FONT_MAX_UNICODE_CHARS - 31;
			}
			Glyph g = LoadedGlyphs[GlyphIndex];
			do
			{
				x += g.TotalSize.X;
			} while (++CharIndex % TAB_SIZE && IsTab);
			MaxHeight = std::max(g.Size.Y + g.Offset.Y, MaxHeight);
			float LastDistance = x / 450 / Graphics::AspectRatio * Scale;
			PrevMaxDepth = x;
			PrevDepth = x;
			if (LastDistance > Dist)
			{
				if (LastDistance > Dist + (g.TotalSize.X / 800 / Graphics::AspectRatio * Scale))
				{
					return std::min(i, TextSegment::CombineToString(Text).size());
				}

				return std::min(i + 1, TextSegment::CombineToString(Text).size());
			}
			PrevMaxDepth = g.Offset.Y + g.Size.X, 0;
			PrevDepth = g.Offset.Y;
		}
		i++;
	}

	return std::min(i, TextSegment::CombineToString(Text).size());
}

TextRenderer::TextRenderer(std::string filename)
{
	if (!std::filesystem::exists(filename))
	{
#if RELEASE
		Filename = "Assets/" + filename;
#else
		Filename = "Fonts/" + filename;
#endif
	}
	else
	{
		Filename = filename;
	}

	if (!std::filesystem::exists(Filename))
	{
		Log::Print("Failed to load font: " + Filename, Log::LogColor::Red);
		return;
	}

	Uint8* ttfBuffer = (Uint8*)malloc(1 << 20);
	ENGINE_ASSERT(ttfBuffer != nullptr, "Failed to allocate space for font bitmap");

	size_t ret = fread(ttfBuffer, 1, 1 << 20, fopen(Filename.c_str(), "rb"));
	if (!ret || !ttfBuffer)
	{
		Log::Print("Failed to load font: " + Filename, Log::LogColor::Red);
		return;
	}
	stbtt_fontinfo FontInfo;
	stbtt_InitFont(&FontInfo, ttfBuffer, stbtt_GetFontOffsetForIndex(ttfBuffer, 0));


	uint8_t* GlypthBitmap = new uint8_t[FONT_BITMAP_WIDTH * FONT_BITMAP_WIDTH](0);
	int Offset = 0;
	int xCoord = 0;
	int yCoord = 0;
	int maxH = 0;

	LoadedGlyphs.clear();

	for (int i = 32; i <= FONT_MAX_UNICODE_CHARS + 1; i++)
	{
		Glyph New;
		int glyph = i;
		if (i > FONT_MAX_UNICODE_CHARS)
		{
			glyph = 0x000025A1;
		}
		int w, h, xOffset, yOffset;
		auto bmp = stbtt_GetCodepointBitmap(&FontInfo,
			0.05f,
			0.05f,
			glyph,
			&w,
			&h,
			&xOffset,
			&yOffset);

		int advW, leftB;
		stbtt_GetCodepointHMetrics(&FontInfo, glyph, &advW, &leftB);

		New.TotalSize.X = (float)advW / 400.0f;
		New.TotalSize.Y = 0;

		if (xCoord + w + FONT_BITMAP_PADDING > FONT_BITMAP_WIDTH)
		{
			xCoord = 0;
			yCoord += maxH + FONT_BITMAP_PADDING;
		}

		New.TexCoordStart = Vector2(
			(float)xCoord / FONT_BITMAP_WIDTH,
			(float)yCoord / FONT_BITMAP_WIDTH);

		New.TexCoordOffset = Vector2(
			(float)(w + 3) / FONT_BITMAP_WIDTH,
			(float)(h + 3) / FONT_BITMAP_WIDTH);

		New.Offset = Vector2((float)xOffset, (float)yOffset) / 20.0;
		New.Size = Vector2((float)w, (float)h) / 20.0;

		CharacterSize = std::max(New.Size.Y + New.Offset.Y, (float)CharacterSize);

		if (New.Size != 0)
		{
			// Give some additional space for better anti aliasing
			New.Size += Vector2(3.0f / 20.0f, 3.0f / 20.0f);
		}

		if (w == 0 || h == 0 || i == ' ')
		{
			New.TexCoordStart = 0;
			New.TexCoordOffset = 0;
			LoadedGlyphs.push_back(New);
			continue;
		}

		for (int ith = 0; ith < h; ith++)
		{
			for (int itw = 0; itw < w; itw++)
			{
				GlypthBitmap[(ith + yCoord) * FONT_BITMAP_WIDTH + (xCoord + itw)] = bmp[ith * w + itw];
			}
		}
		maxH = std::max(maxH, h);
		xCoord += w + FONT_BITMAP_PADDING;
		LoadedGlyphs.push_back(New);
		free(bmp);
	}

	glGenTextures(1, &fontTexture);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_ALPHA,
		FONT_BITMAP_WIDTH,
		FONT_BITMAP_WIDTH,
		0,
		GL_ALPHA,
		GL_UNSIGNED_BYTE,
		GlypthBitmap);
	// can free temp_bitmap at this point
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glGenVertexArrays(1, &fontVao);
	glBindVertexArray(fontVao);
	glGenBuffers(1, &fontVertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, fontVertexBufferId);

	fontVertexBufferCapacity = 35;
	fontVertexBufferData = new FontVertex[fontVertexBufferCapacity * 6];

	glBufferData(GL_ARRAY_BUFFER, sizeof(FontVertex) * 6 * fontVertexBufferCapacity, 0, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(FontVertex), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(FontVertex), (const void*)offsetof(FontVertex, texCoords));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(FontVertex), (const void*)offsetof(FontVertex, color));
	glBindVertexArray(0);

	free(ttfBuffer);
	delete[] GlypthBitmap;
}

Vector2 TextRenderer::GetTextSize(ColoredText Text, float Scale, bool Wrapped, float LengthBeforeWrap)
{
	Scale *= 5.0f;
	LengthBeforeWrap = LengthBeforeWrap * Graphics::AspectRatio / Scale;
	float x = 0.f, y = CharacterSize * 5;
	float MaxX = 0.0f;
	FontVertex* vData = fontVertexBufferData;
	Uint32 numVertices = 0;
	size_t Wraps = 0;
	size_t CharIndex = 0;
	for (auto& seg : Text)
	{
		size_t LastWordIndex = SIZE_MAX;
		size_t LastWrapIndex = 0;
		Uint32 LastWordNumVertices = 0;
		FontVertex* LastWordVDataPtr = nullptr;
		std::wstring SegmentText = GetUnicodeString(seg.Text);
		for (size_t i = 0; i < SegmentText.size(); i++)
		{
			bool IsTab = SegmentText[i] == '\t';
			if (IsTab)
			{
				SegmentText[i] = ' ';
			}

			if (SegmentText[i] >= 32)
			{
				int GlyphIndex = (int)SegmentText[i] - 32;
				if (GlyphIndex < 0)
				{
					continue;
				}
				if (GlyphIndex > FONT_MAX_UNICODE_CHARS)
				{
					GlyphIndex = FONT_MAX_UNICODE_CHARS - 31;
				}
				Glyph g = LoadedGlyphs[GlyphIndex];
				do
				{
					x += g.TotalSize.X;
				} while (++CharIndex % TAB_SIZE && IsTab);

				if (SegmentText[i] == ' ')
				{
					LastWordIndex = i;
					LastWordNumVertices = numVertices;
					LastWordVDataPtr = vData;
				}

				vData += 6;
				numVertices += 6;
				MaxX = std::max(MaxX, x);
			}
			if ((x / 225 > LengthBeforeWrap && Wrapped) || SegmentText[i] == (int)'\n')
			{
				Wraps++;
				if (LastWordIndex != SIZE_MAX && LastWordIndex != LastWrapIndex)
				{
					i = LastWordIndex;
					LastWrapIndex = i;
					vData = LastWordVDataPtr;
					numVertices = LastWordNumVertices;
				}
				x = 0;
				y += CharacterSize * 5;
			}
		}
	}
	return Vector2(MaxX / 450 / Graphics::AspectRatio * Scale, y / 450 * Scale);
}

DrawableText* TextRenderer::MakeText(ColoredText Text, Vector2 Pos, float Scale, Vector3 Color, float opacity, float LengthBeforeWrap)
{
	size_t CharIndex = 0;
	for (auto& i : Text)
	{
		for (size_t it = 0; it < i.Text.size(); it++)
		{
			if (i.Text[it] == '\t')
			{
				i.Text[it] = ' ';
				while (++CharIndex % TAB_SIZE)
				{
					i.Text.insert(i.Text.begin() + it++, ' ');
				}
			}
			else
			{
				CharIndex++;
			}
		}
	}

	GLuint newVAO = 0, newVBO = 0;
	glGenVertexArrays(1, &newVAO);
	glBindVertexArray(newVAO);
	glGenBuffers(1, &newVBO);
	glBindBuffer(GL_ARRAY_BUFFER, newVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(FontVertex) * 6 * fontVertexBufferCapacity, 0, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(FontVertex), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(FontVertex), (const void*)offsetof(FontVertex, texCoords));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(FontVertex), (const void*)offsetof(FontVertex, color));
	glBindVertexArray(0);

	Scale *= 5.0f;
	LengthBeforeWrap = LengthBeforeWrap * Graphics::AspectRatio / Scale;
	Pos.X = Pos.X * 450 * Graphics::AspectRatio;
	Pos.Y = Pos.Y * -450;
	glBindVertexArray(newVAO);
	glBindBuffer(GL_ARRAY_BUFFER, newVBO);
	uint32_t len = (uint32_t)TextSegment::CombineToString(Text).size();
	if (fontVertexBufferCapacity < len)
	{
		fontVertexBufferCapacity = len;
		glBufferData(GL_ARRAY_BUFFER, sizeof(FontVertex) * 6 * fontVertexBufferCapacity, 0, GL_DYNAMIC_DRAW);
		delete[] fontVertexBufferData;
		fontVertexBufferData = new FontVertex[fontVertexBufferCapacity * 6];
	}
	float x = 0.f, y = 0.f;
	FontVertex* vData = fontVertexBufferData;
	Uint32 numVertices = 0;
	for (auto& seg : Text)
	{
		size_t LastWordIndex = SIZE_MAX;
		size_t LastWrapIndex = 0;
		Uint32 LastWordNumVertices = 0;
		FontVertex* LastWordVDataPtr = nullptr;
		std::wstring UTFString = GetUnicodeString(seg.Text);

		for (size_t i = 0; i < UTFString.size(); i++)
		{
			int GlyphIndex = (int)UTFString[i] - 32;
			if (GlyphIndex >= 0)
			{
				if (GlyphIndex > FONT_MAX_UNICODE_CHARS)
				{
					GlyphIndex = FONT_MAX_UNICODE_CHARS - 31;
				}
				Glyph g = LoadedGlyphs[GlyphIndex];

				Vector2 StartPos = Vector2(x, y) + g.Offset;
				if (g.Size != 0)
				{
					vData[0].position = StartPos + Vector2(0, g.Size.Y); vData[0].texCoords = g.TexCoordStart + Vector2(0, g.TexCoordOffset.Y);
					vData[1].position = StartPos + g.Size;               vData[1].texCoords = g.TexCoordStart + g.TexCoordOffset;
					vData[2].position = StartPos + Vector2(g.Size.X, 0); vData[2].texCoords = g.TexCoordStart + Vector2(g.TexCoordOffset.X, 0);
					vData[3].position = StartPos;                        vData[3].texCoords = g.TexCoordStart;
					vData[4].position = StartPos + Vector2(0, g.Size.Y); vData[4].texCoords = g.TexCoordStart + Vector2(0, g.TexCoordOffset.Y);
					vData[5].position = StartPos + Vector2(g.Size.X, 0); vData[5].texCoords = g.TexCoordStart + Vector2(g.TexCoordOffset.X, 0);
					vData[0].color = seg.Color;	                         vData[1].color = seg.Color;
					vData[2].color = seg.Color;	                         vData[3].color = seg.Color;
					vData[4].color = seg.Color;                          vData[5].color = seg.Color;
					vData += 6;
					numVertices += 6;
				}
				x += g.TotalSize.X;

				if (UTFString[i] == ' ')
				{
					LastWordIndex = i;
					LastWordNumVertices = numVertices;
					LastWordVDataPtr = vData;
				}
			}
			if (x / 225 > LengthBeforeWrap || UTFString[i] == (int)'\n')
			{
				if (LastWordIndex != SIZE_MAX && LastWordIndex != LastWrapIndex)
				{
					i = LastWordIndex;
					LastWrapIndex = i;
					vData = LastWordVDataPtr;
					numVertices = LastWordNumVertices;
				}
				x = 0;
				y += CharacterSize * 5.0f;
			}
		}
	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(FontVertex) * numVertices, fontVertexBufferData);
	return new DrawableText(newVAO, newVBO, numVertices, fontTexture, Pos, Scale, Color, opacity);
}

TextRenderer::~TextRenderer()
{
	glDeleteTextures(1, &fontTexture);
	glDeleteBuffers(1, &fontVertexBufferId);
	glDeleteBuffers(1, &fontVao);
	if (fontVertexBufferData)
	{
		delete[] fontVertexBufferData;
	}
}

DrawableText::DrawableText(unsigned int VAO, unsigned int VBO, unsigned int NumVerts, unsigned int Texture,
	Vector2 Position, float Scale, Vector3 Color, float opacity)
{
	this->Position = Position;
	this->Scale = Scale;
	this->NumVerts = NumVerts;
	this->VAO = VAO;
	this->VBO = VBO;
	this->Opacity = opacity;
	this->Texture = Texture;
	this->Color = Color;
}

void DrawableText::Draw(ScrollObject* CurrentScrollObject) const
{
	Shader* TextShader = Graphics::TextShader;
	glBindVertexArray(VAO);
	TextShader->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture);
	TextShader->SetInt("u_texture", 0);
	TextShader->SetVector3("textColor", Vector3(Color.X, Color.Y, Color.Z));
	TextShader->SetFloat("u_aspectratio", Graphics::AspectRatio);
	TextShader->SetVector3("transform", Vector3((float)Position.X, (float)Position.Y, Scale));
	TextShader->SetVector2("u_screenRes", Vector2(Graphics::WindowResolution.Y * 1.5f));
	TextShader->SetFloat("u_opacity", Opacity);
	if (CurrentScrollObject != nullptr)
	{
		TextShader->SetVector3("u_offset",
			Vector3(-CurrentScrollObject->Percentage, CurrentScrollObject->Position.Y, CurrentScrollObject->Position.Y - CurrentScrollObject->Scale.Y));
	}
	else
		TextShader->SetVector3("u_offset", Vector3(0, -1000, 1000));
	glDrawArrays(GL_TRIANGLES, 0, NumVerts);
}


DrawableText::~DrawableText()
{
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}
#endif