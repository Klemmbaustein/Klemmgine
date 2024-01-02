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

	void Update() override;

	float Rotation = 0;
	Vector3 Color = 1;
protected:
	unsigned int LoadedTexture = 0;
	BillboardSprite* Sprite = nullptr;
};