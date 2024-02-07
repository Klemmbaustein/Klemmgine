#include "BillboardComponent.h"
#include <Rendering/BillboardSprite.h>
#include <Rendering/Texture/Texture.h>
#include <Rendering/Graphics.h>
#include <Engine/Log.h>

BillboardComponent::BillboardComponent()
{
}

BillboardComponent::~BillboardComponent()
{
#if !SERVER
	if (LoadedTexture)
	{
		Texture::UnloadTexture(LoadedTexture);
	}
	if (Sprite)
	{
		delete Sprite;
	}
#endif
}

void BillboardComponent::Load(std::string Texture)
{
#if !SERVER
	Load(Texture::LoadTexture(Texture, Texture::TextureFiltering::Linear));
#endif
}

void BillboardComponent::Load(unsigned int Texture)
{
#if !SERVER
	Sprite = new BillboardSprite(Texture, Graphics::MainFramebuffer);
}

BillboardSprite* BillboardComponent::GetSprite()
{
	return Sprite;
#endif
}

void BillboardComponent::Update()
{
#if !SERVER
	if (!Sprite) return;
	if (!GetParent()) return;
	Sprite->Position = GetParent()->GetTransform().Position + RelativeTransform.Position;
	Sprite->Rotation = Rotation;
	Sprite->Color = Color;
#endif
}
