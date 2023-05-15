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
//class ScrollObject;
class Shader;
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
	void Draw(ScrollObject* CurrentScrollObject);
	~DrawableText();
};

class TextRenderer
{
	friend class DrawableText;
private:
	void* cdatapointer;
	unsigned int fontTexture;
	unsigned int fontVao;
	unsigned int fontVertexBufferId;
	FontVertex* fontVertexBufferData = 0;
	uint32_t fontVertexBufferCapacity;
public:
	std::string Filename; float CharacterSizeInPixels;
	size_t GetCharacterIndexADistance(ColoredText Text, float Dist, float Scale, Vector2& LetterOutLocation);
	TextRenderer(std::string filename = "Font.ttf", float CharacterSizeInPixels = 150);
	Vector2 GetTextSize(ColoredText Text, float Scale, bool Wrapped, float LengthBeforeWrap);
	Vector2 RenderText(ColoredText Text, Vector2 Pos, float Scale, Vector3 Color, float opacity, float LengthBeforeWrap, ScrollObject* CurrentScrollObject);
	DrawableText* MakeText(ColoredText Text, Vector2 Pos, float Scale, Vector3 Color, float opacity, float LengthBeforeWrap);
	void Reinit();
	~TextRenderer();

};