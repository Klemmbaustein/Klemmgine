#pragma once
#include <Rendering/Renderable.h>

class Camera;
class FramebufferObject;
struct VertexBuffer;

class BillboardSprite : public Renderable
{
public:

	BillboardSprite(unsigned int Texture, FramebufferObject* Buffer);
	virtual ~BillboardSprite();

	float Size = 1;
	Vector3 Position;
	float Rotation = 0;
	unsigned int TextureObject = 0;
	bool ScaleWithDistance = true;
	Vector3 Color = 1;
	float Opacity = 1;

	void Render(Camera* WorldCamera, bool MainFrameBuffer, bool TransparencyPass) override;
	void SimpleRender(Shader* UsedShader) override;
protected:

	FramebufferObject* ParentBuffer;
	Vector3 ScreenPosition;
	VertexBuffer* BillboardVertexBuffer = nullptr;
	static Shader* BillboardShader;
};