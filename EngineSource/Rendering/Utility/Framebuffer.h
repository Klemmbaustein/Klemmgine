#pragma once
#include <vector>
#include <Rendering/Renderable.h>
#include <Rendering/Particle.h>

class Framebuffer
{
public:

	void AttachFramebuffer(unsigned int Buffer, unsigned int Attachement = 0x8CE3);

	Framebuffer();
	~Framebuffer();

	void Bind();
	void Unbind();
	unsigned int GetTextureID(int Index);

	void ReInit(int Width, int Height, bool ColorAttachementType = false);
	
protected:
	unsigned int FBO = 0;
	unsigned int numbuffers = 4;
	unsigned int* Textures = new unsigned int[4];
	std::vector<unsigned int> Attachements = { 0x8CE0, 0x821A, 0x8CE1, 0x8CE2 };

};

class FramebufferObject
{
public:
	bool UseMainWindowResolution = true;
	Vector2 CustomFramebufferResolution = Vector2(800, 600);
	FramebufferObject();
	~FramebufferObject();

	unsigned int GetTextureID();
	void ClearContent();
	Camera* FramebufferCamera = nullptr;
	void ReInit();
	void UseWith(Renderable* r);
	std::vector<Particles::ParticleEmitter*> ParticleEmitters;
	Framebuffer* GetBuffer();
	std::string ReflectionCubemapName;
	std::string PreviousReflectionCubemapName;
	unsigned int ReflectionCubemap = 0;
	std::vector<Renderable*> Renderables;
protected:
	Framebuffer* buf;
};