#include "Bloom.h"
#include "PostProcess.h"
#include "SSAO.h"
#include <Engine/Application.h>
#include <Engine/EngineProperties.h>
#include <Engine/Stats.h>
#include <Engine/Subsystem/Console.h>
#include <GL/glew.h>
#include <Math/Collision/CollisionVisualize.h>
#include <Rendering/Framebuffer.h>
#include <Rendering/Graphics.h>
#include <Rendering/Shader.h>
#include <UI/UIBox.h>
#include <UI/EditorUI/Viewport.h>

std::vector<PostProcess::Effect*> PostProcess::AllEffects;
PostProcess* PostProcess::PostProcessSystem = nullptr;

unsigned int PostProcess::Effect::Render(unsigned int TargetBuffer) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, EffectBuffer);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	EffectShader->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TargetBuffer);
	EffectShader->SetInt("u_texture", 0);
	EffectShader->SetInt("u_hasUITexCoords", 0);
	EffectShader->SetVector2("Position", 0);
	EffectShader->SetVector2("u_screenRes", Graphics::RenderResolution);
	EffectShader->SetFloat("u_time", Stats::Time);

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 3);


	return EffectTexture;
}

PostProcess::Effect::Effect(std::string FragmentShader, EffectType UsedType)
{
	this->UsedType = UsedType;
	EffectShader = new Shader("Shaders/Internal/postprocess.vert", "Shaders/" + FragmentShader);
	glGenFramebuffers(1, &EffectBuffer);
	glGenTextures(1, &EffectTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, EffectBuffer);
	glBindTexture(GL_TEXTURE_2D, EffectTexture);

	Vector2 Resolution = (UsedType == EffectType::UI || UsedType == EffectType::UI_Internal) ? Graphics::WindowResolution : Graphics::RenderResolution;

	int SizeX = (int)(Resolution.X), SizeY = (int)(Resolution.Y);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA16F,
		SizeX,
		SizeY,
		0,
		GL_RGBA,
		GL_FLOAT,
		NULL
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, EffectTexture, 0
	);
}

void PostProcess::Effect::UpdateSize()
{
	glDeleteFramebuffers(1, &EffectBuffer);
	glDeleteTextures(1, &EffectTexture);
	glGenFramebuffers(1, &EffectBuffer);
	glGenTextures(1, &EffectTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, EffectBuffer);
	glBindTexture(GL_TEXTURE_2D, EffectTexture);

	Vector2 Resolution = (UsedType == EffectType::UI || UsedType == EffectType::UI_Internal) ? Graphics::WindowResolution : Graphics::RenderResolution;

	int SizeX = (int)(Resolution.X), SizeY = (int)(Resolution.Y);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA16F,
		SizeX,
		SizeY,
		0,
		GL_RGBA,
		GL_FLOAT,
		NULL
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, EffectTexture, 0
	);
}

PostProcess::PostProcess()
{
	Name = "PostProcess";
	PostProcessSystem = this;

	PostProcessShader = new Shader("Shaders/Internal/postprocess.vert", "Shaders/Internal/postprocess.frag");

	UIMergeEffect = new PostProcess::Effect("Internal/uimerge.frag", PostProcess::EffectType::UI_Internal);
	AddEffect(UIMergeEffect);

	AntiAliasEffect = new PostProcess::Effect("Internal/fxaa.frag", PostProcess::EffectType::World_Internal);
	AddEffect(AntiAliasEffect);

	Console::ConsoleSystem->RegisterConVar(Console::Variable("post_process", NativeType::Bool, &RenderPostProcess, nullptr));
	Console::ConsoleSystem->RegisterConVar(Console::Variable("full_bright", NativeType::Bool, &Graphics::RenderFullbright, nullptr));
	Console::ConsoleSystem->RegisterConVar(Console::Variable("aa_enabled", NativeType::Bool, &Graphics::RenderAntiAlias, []() {
		Graphics::SetWindowResolution(Graphics::WindowResolution, true);
		}));
}

void PostProcess::AddEffect(Effect* NewEffect)
{
	AllEffects.push_back(NewEffect);
}

bool PostProcess::RemoveEffect(Effect*& TargetEffect)
{
	for (size_t i = 0; i < AllEffects.size(); i++)
	{
		if (AllEffects[i] == TargetEffect)
		{
			AllEffects.erase(AllEffects.begin() + i);
			delete TargetEffect;
			TargetEffect = nullptr;
			return true;
		}
	}
	return false;
}

const std::vector<PostProcess::Effect*> PostProcess::GetCurrentEffects()
{
	return PostProcess::AllEffects;
}

void PostProcess::Update()
{
}

void PostProcess::Draw()
{
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

#if !SERVER
	bool ShouldSkip3D = false;
#if EDITOR
	if (!Application::WindowHasFocus())
	{
		ShouldSkip3D = true;
	}
#endif
	glViewport(0, 0, (int)Graphics::WindowResolution.X, (int)Graphics::WindowResolution.Y);
	UIMergeEffect->EffectShader->Bind();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, UIBox::GetUITextures()[1]);
	UIMergeEffect->EffectShader->SetInt("u_alpha", 1);
	UIMergeEffect->EffectShader->SetInt("u_hr", 2);
	UIMergeEffect->EffectShader->SetInt("u_hrAlpha", 3);
	unsigned int UIBuffer = UIMergeEffect->Render(UIBox::GetUITextures()[0]);

#if !EDITOR
	for (const PostProcess::Effect* i : PostProcess::GetCurrentEffects())
	{
		if (i->UsedType == PostProcess::EffectType::UI)
		{
			UIBuffer = i->Render(UIBuffer);
		}
	}
#endif

	FramebufferObject* DrawnBuffer = CollisionVisualize::GetVisualizeBuffer()
		? CollisionVisualize::GetVisualizeBuffer()
		: Graphics::MainFramebuffer;

	if (!ShouldSkip3D)
	{
		glViewport(0, 0, (int)Graphics::RenderResolution.X, (int)Graphics::RenderResolution.Y);
		AOBuffer = SSAO::Render(
			DrawnBuffer->GetBuffer()->GetTextureID(2),
			DrawnBuffer->GetBuffer()->GetTextureID(3));

		MainPostProcessBuffer = DrawnBuffer->GetBuffer()->GetTextureID(0);

		if (RenderPostProcess)
		{
			if (Graphics::RenderAntiAlias)
			{
				MainPostProcessBuffer = AntiAliasEffect->Render(MainPostProcessBuffer);
			}
			for (const PostProcess::Effect* i : PostProcess::GetCurrentEffects())
			{
				if (i->UsedType == PostProcess::EffectType::World)
				{
					MainPostProcessBuffer = i->Render(MainPostProcessBuffer);
				}
			}
		}
		BloomBuffer = Bloom::BlurFramebuffer(MainPostProcessBuffer);
	}

	glViewport(0, 0, (int)Graphics::WindowResolution.X, (int)Graphics::WindowResolution.Y);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Stats::EngineStatus = "Rendering (Post process: Main)";

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, MainPostProcessBuffer);
#if EDITOR
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, Viewport::ViewportInstance->OutlineBuffer->GetBuffer()->GetTextureID(1));
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, Viewport::ViewportInstance->ArrowsBuffer->GetBuffer()->GetTextureID(0));
#else
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);

#endif
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, BloomBuffer);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, AOBuffer);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, DrawnBuffer->GetBuffer()->GetTextureID(1));

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, UIBuffer);
	PostProcessShader->Bind();
	PostProcessShader->SetFloat("u_gamma", Graphics::Gamma);
#if EDITOR
	const Vector2 ViewportPos = (Viewport::ViewportInstance->Position + Viewport::ViewportInstance->Scale * 0.5);
	PostProcessShader->SetVector2("Position", ViewportPos);
	const Vector2 ViewportScale = 2;
	PostProcessShader->SetVector2("Scale", ViewportScale);
#endif
	PostProcessShader->SetInt("u_texture", 1);
	PostProcessShader->SetInt("u_outlines", 2);
	PostProcessShader->SetInt("u_enginearrows", 3);
	PostProcessShader->SetInt("u_ssaotexture", 5);
	PostProcessShader->SetInt("u_depth", 6);
	PostProcessShader->SetInt("u_hasUITexCoords", 1);
	PostProcessShader->SetInt("u_ui", 7);
	PostProcessShader->SetInt("u_uialpha", 8);
	PostProcessShader->SetFloat("u_time", Stats::Time);
	PostProcessShader->SetFloat("u_vignette", Graphics::Vignette);
	PostProcessShader->SetInt("u_bloom", Graphics::Bloom);
	PostProcessShader->SetInt("u_ssao", Graphics::SSAO);
	PostProcessShader->SetInt("u_editor", IS_IN_EDITOR);
	if (Graphics::Bloom)
	{
		PostProcessShader->SetInt("u_bloomtexture", 4);
	}
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	PostProcessShader->Unbind();
	glViewport(0, 0, (int)Graphics::RenderResolution.X, (int)Graphics::RenderResolution.Y);
#endif
}
