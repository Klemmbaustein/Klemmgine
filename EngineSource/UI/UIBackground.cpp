#if !SERVER
#include "UIBackground.h"
#include <GL/glew.h>
#include <Rendering/Vertex.h>
#include <Rendering/VertexBuffer.h>
#include <Rendering/Shader.h>
#include <Engine/Log.h>
#include <Rendering/Graphics.h>
#include <Rendering/Texture/Texture.h>

void UIBackground::ScrollTick(Shader* UsedShader)
{
	if (CurrentScrollObject != nullptr)
	{
		glUniform3f(glGetUniformLocation(UsedShader->GetShaderID(), "u_offset"),
			-CurrentScrollObject->Percentage,
			CurrentScrollObject->Position.Y,
			CurrentScrollObject->Position.Y - CurrentScrollObject->Scale.Y);
	}
	else
	{
		glUniform3f(glGetUniformLocation(UsedShader->GetShaderID(), "u_offset"), 0, -1000, 1000);
	}
}

void UIBackground::MakeGLBuffers(bool InvertTextureCoordinates)
{
	if(BoxVertexBuffer)
	delete BoxVertexBuffer;
	std::vector<Vertex> Vertices;
	Vertex vertex1;
	vertex1.Position.x = 0;
	vertex1.Position.y = 0;
	vertex1.Position.z = 0;
	vertex1.TexCoord = glm::vec2(InvertTextureCoordinates, 0);
	Vertices.push_back(vertex1);

	Vertex vertex2;
	vertex2.Position.x = 1;
	vertex2.Position.y = 0;
	vertex2.Position.z = 0;
	vertex2.TexCoord = glm::vec2(!InvertTextureCoordinates, 0);
	Vertices.push_back(vertex2);

	Vertex vertex3;
	vertex3.Position.x = 0;
	vertex3.Position.y = 1;
	vertex3.Position.z = 0;
	vertex3.TexCoord = glm::vec2(InvertTextureCoordinates, 1);
	Vertices.push_back(vertex3);

	Vertex vertex4;
	vertex4.Position.x = 1;
	vertex4.Position.y = 1;
	vertex4.Position.z = 0;
	vertex4.TexCoord = glm::vec2(!InvertTextureCoordinates, 1);
	Vertices.push_back(vertex4);
	BoxVertexBuffer = new VertexBuffer(Vertices, {0, 1, 2, 1, 2, 3});
}

UIBackground* UIBackground::SetOpacity(float NewOpacity)
{
	if (NewOpacity != Opacity)
	{
		Opacity = NewOpacity;
		RedrawUI();
	}
	return this;
}

float UIBackground::GetOpacity() const
{
	return Opacity;
}

void UIBackground::SetColor(Vector3 NewColor)
{
	if (NewColor != Color)
	{
		Color = Vector3::Clamp(NewColor, 0, 1);
		RedrawUI();
	}
}

Vector3 UIBackground::GetColor() const
{
	return Color;
}

bool UIBackground::GetUseTexture() const
{
	return TextureMode;
}

void UIBackground::SetInvertTextureCoordinates(bool Invert)
{
	MakeGLBuffers(Invert);
}

UIBackground* UIBackground::SetUseTexture(bool UseTexture, unsigned int TextureID)
{
	if (this->TextureMode != (UseTexture ? 1 : 0) || TextureID != this->TextureID)
	{
		this->TextureMode = UseTexture ? 1 : 0;
		this->TextureID = TextureID;
		RedrawUI();
	}

	return this;
}

UIBackground* UIBackground::SetUseTexture(bool UseTexture, std::string TextureName)
{
	this->TextureMode = (UseTexture ? 2 : 0);
	if (!TextureMode)
	{
		return this;
	}
	this->TextureID = Texture::LoadTexture(TextureName);
	if (TextureID == 0)
	{
		TextureMode = 0;
	}
	RedrawUI();

	return this;
}

UIBackground::UIBackground(bool Horizontal, Vector2 Position, Vector3 Color, Vector2 MinScale, Shader* UsedShader) : UIBox(Horizontal, Position)
{
	SetMinSize(MinScale);
	this->Color = Vector3::Clamp(Color, 0, 1);
	this->BackgroundShader = UsedShader;
	MakeGLBuffers();
}

UIBackground::~UIBackground()
{
	delete BoxVertexBuffer;

	if (TextureMode == 2)
	{
		Texture::UnloadTexture(TextureID);
	}
}

void UIBackground::Draw()
{
	BackgroundShader->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureID);
	BoxVertexBuffer->Bind();
	ScrollTick(BackgroundShader);
	glUniform1i(glGetUniformLocation(BackgroundShader->GetShaderID(), "u_texture"), 0);
	glUniform4f(glGetUniformLocation(BackgroundShader->GetShaderID(), "u_color"), Color.X, Color.Y, Color.Z, 1.f);
	glUniform4f(glGetUniformLocation(BackgroundShader->GetShaderID(), "u_transform"), OffsetPosition.X, OffsetPosition.Y, Size.X, Size.Y);
	BackgroundShader->SetFloat("u_opacity", Opacity);
	BackgroundShader->SetInt("u_borderType", (int)BoxBorder);
	BackgroundShader->SetFloat("u_borderScale", BorderRadius / 20.0f);
	BackgroundShader->SetFloat("u_aspectratio", Graphics::AspectRatio);
	BackgroundShader->SetFloat("u_depth", GetCurrentUIDepth());

	BackgroundShader->SetInt("u_useTexture", (bool)TextureMode);
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	BoxVertexBuffer->Unbind();
}

void UIBackground::Update()
{
}

void UIBackground::OnAttached()
{
}
#endif