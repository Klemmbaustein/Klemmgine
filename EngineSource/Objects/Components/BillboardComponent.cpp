#include "BillboardComponent.h"
#include <Rendering/BillboardSprite.h>
#include <Rendering/Texture/Texture.h>
#include <World/Graphics.h>
#include <Engine/Log.h>

BillboardComponent::BillboardComponent()
{
}

BillboardComponent::~BillboardComponent()
{
	if (LoadedTexture)
	{
		Texture::UnloadTexture(LoadedTexture);
	}
	if (Sprite)
	{
		delete Sprite;
	}
}

void BillboardComponent::Load(std::string Texture)
{
	Load(Texture::LoadTexture(Texture));
}

void BillboardComponent::Load(unsigned int Texture)
{
	Sprite = new BillboardSprite(Texture, Graphics::MainFramebuffer);
}

BillboardSprite* BillboardComponent::GetSprite()
{
	return Sprite;
}

void BillboardComponent::Tick()
{
	if (!Sprite) return;
	if (!GetParent()) return;
	Sprite->Position = GetParent()->GetTransform().Location + RelativePosition;
	Sprite->Rotation = Rotation;
}
