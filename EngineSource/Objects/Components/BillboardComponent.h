#pragma once
#include <Objects/Components/Component.h>

class BillboardSprite;

class BillboardComponent : public Component
{
public:
	BillboardComponent();
	~BillboardComponent();

	void Load(std::string Texture);
	void Load(unsigned int Texture);

	BillboardSprite* GetSprite();

	void Tick() override;

	float Rotation = 0;
	Vector3 RelativePosition;
protected:
	unsigned int LoadedTexture = 0;
	BillboardSprite* Sprite = nullptr;
};