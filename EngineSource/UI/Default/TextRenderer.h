#if !SERVER
#pragma once
#include <Math/Vector.h>
#include "glm/ext.hpp"
#include <UI/Default/TextSegment.h>

class ScrollObject;

struct FontVertex
{
	Vector2 position;
	Vector2 texCoords;
	Vector3 color = 1;
};
struct Shader;

class DrawableText
{
	friend class TextRenderer;
	unsigned int VAO, VBO;
	unsigned int Texture;
	Vector3 Color;
	//ScrollObject* CurrentScrollObject;
	float Scale;
	Vector2 Position;
	unsigned int NumVerts;
	DrawableText(unsigned int VAO, unsigned int VBO, unsigned int NumVerts, unsigned int Texture, Vector2 Position, float Scale, Vector3 Color, float opacity);
public:
	float Opacity = 1.f;
	void Draw(ScrollObject* CurrentScrollObject) const;
	~DrawableText();
};

class TextRenderer
{
	friend class DrawableText;
private:
	unsigned int fontTexture = 0;
	unsigned int fontVao = 0;
	unsigned int fontVertexBufferId = 0;
	FontVertex* fontVertexBufferData = 0;
	uint32_t fontVertexBufferCapacity = 0;
public:
	float CharacterSize = 0;
	struct Glyph
	{
		Vector2 Size;
		Vector2 Offset;
		Vector2 TotalSize;
		Vector2 TexCoordStart;
		Vector2 TexCoordOffset;
	};
	std::vector<Glyph> LoadedGlyphs;

	std::string Filename;
	size_t GetCharacterIndexADistance(ColoredText Text, float Dist, float Scale);
	TextRenderer(std::string filename = "Font.ttf");
	Vector2 GetTextSize(ColoredText Text, float Scale, bool Wrapped, float LengthBeforeWrap);
	Vector2 GetLetterPosition(ColoredText Text, size_t Index, float Scale, bool Wrapped, float LengthBeforeWrap);
	DrawableText* MakeText(ColoredText Text, Vector2 Pos, float Scale, Vector3 Color, float opacity, float LengthBeforeWrap);
	~TextRenderer();

};
#endif