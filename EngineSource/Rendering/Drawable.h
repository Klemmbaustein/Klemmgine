#pragma once
#include <Rendering/Camera/Camera.h>
#include <Rendering/Shader.h>
#include <Rendering/Texture/Material.h>
#include <Rendering/Texture/Texture.h>

struct ObjectRenderContext
{
	ObjectRenderContext(Material m);
	ObjectRenderContext();
	~ObjectRenderContext();
	void Bind();
	void BindWithShader(Shader* s);
	struct Uniform
	{
		std::string Name;
		NativeType::NativeType NativeType;
		void* Content;
		Uniform(std::string Name, NativeType::NativeType NativeType, void* Content)
		{
			this->Content = Content;
			this->Name = Name;
			this->NativeType = NativeType;
		}
	};

	Shader* GetShader();
	Material Mat;

	void LoadUniform(Material::Param u);
	void LoadTexture(std::string TextureName, Texture::TextureType TextureID);
	void Unload();

	bool DeleteUniform(std::string Name);

protected:
	Shader* ContextShader = nullptr;
	std::vector<Uniform> Uniforms;
};

class Drawable
{
public:
	virtual void Render(Camera* WorldCamera, bool MainFrameBuffer, bool TransparencyPass) = 0;
	virtual void SimpleRender(Shader* UsedShader) = 0;
	Drawable()
	{

	};
	virtual ~Drawable()
	{

	};

	static void ApplyDefaultUniformsToShader(Shader* ShaderToApply, bool MainFramebuffer);

	bool CastShadow = true;
	bool DestroyOnUnload = true;
};
