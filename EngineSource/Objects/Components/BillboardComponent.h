#pragma once
#include <Objects/Components/Component.h>
#include <Rendering/Texture/Texture.h>

class BillboardSprite;

/**
* @brief
* A billboard sprite component.
* 
* Renders a 2d sprite in 3d space that will always face the camera.
* 
* @ingroup Components
*/
class BillboardComponent : public Component
{
public:
	BillboardComponent();
	~BillboardComponent();

	/**
	* @brief
	* Loads a texture file for the sprite.
	* 
	* @param Texture
	* The texture file name, without the extension. Content/Sprite.png -> Sprite
	* 
	* @param Filtering
	* The filtering used for the sprite.
	*/
	void Load(std::string Texture, Texture::TextureFiltering Filtering = Texture::TextureFiltering::Linear);
	/**
	* @brief
	* Loads an already existing texture for the sprite.
	*/
	void Load(Texture::TextureType Texture);

	BillboardSprite* GetSprite();

	void Update() override;

	/// The sprite's rotation.
	float Rotation = 0;
	/// The sprite's color.
	Vector3 Color = 1;
protected:
	bool OwnsTexture = false;
	Texture::TextureType LoadedTexture = 0;
	BillboardSprite* Sprite = nullptr;
};