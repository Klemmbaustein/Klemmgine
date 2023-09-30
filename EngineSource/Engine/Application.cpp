// Engine includes
#include <Engine/Application.h>
#include <Engine/OS.h>
#include <Engine/Input.h>
#include <Engine/EngineProperties.h>
#include <Engine/Log.h>
#include <Engine/Scene.h>
#include <Engine/Console.h>
#include <Engine/EngineError.h>
#include <Engine/File/Assets.h>
#include <Engine/Stats.h>
#include <Sound/Sound.h>

#include <UI/Default/UICanvas.h>
#include <UI/UIBox.h>
#include <UI/Default/TextRenderer.h>
#include <UI/EditorUI/EditorUI.h>
#include <UI/EditorUI/Viewport.h>
#include <UI/Debug/DebugUI.h>

#include <Rendering/Renderable.h>
#include <Rendering/Shader.h>
#include <Rendering/Particle.h>
#include <Rendering/Camera/CameraShake.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Rendering/Utility/CSM.h>
#include <Rendering/Utility/SSAO.h>
#include <Rendering/Utility/Bloom.h>
#include <Rendering/Utility/BakedLighting.h>
#include <Rendering/Texture/Cubemap.h>
#include <Rendering/Camera/Camera.h>
#include <Rendering/Camera/FrustumCulling.h>
#include <Rendering/Graphics.h>
#include <Rendering/Utility/PostProcess.h>

#include <Math/Collision/Collision.h>

#include <CSharp/CSharpInterop.h>

// Library includes
#include <GL/glew.h>
#include <SDL.h>

// STL includes
#include <iostream>
#include <thread>

#include <Rendering/BillboardSprite.h>
#include <Rendering/Texture/Texture.h>

Vector2 GetMousePosition()
{
	int x;
	int y;
	SDL_GetMouseState(&x, &y);
	Vector2 Size = Application::GetWindowSize();
	return Vector2((x / Size.X - 0.5f) * 2, 1 - (y / Size.Y * 2));
}

namespace Application
{
	PostProcess::Effect* UIMergeEffect = nullptr;
	std::string StartupSceneOverride;
	unsigned int BloomBuffer = 0, AOBuffer = 0;
	unsigned int MainPostProcessBuffer = 0;
	bool ShowStartupInfo = true;
	bool RenderPostProcess = true;

#if EDITOR
	constexpr int MOUSE_GRAB_PADDING = 5;

	SDL_HitTestResult HitTestCallback(SDL_Window* Window, const SDL_Point* Area, void* Data)
	{
		int Width, Height;
		SDL_GetWindowSize(Window, &Width, &Height);
		int x;
		int y;
		SDL_GetMouseState(&x, &y);
		Input::MouseLocation = Vector2(((float)Area->x / (float)Width - 0.5f) * 2.0f, 1.0f - ((float)Area->y / (float)Height * 2.0f));

		if (Area->y < MOUSE_GRAB_PADDING)
		{
			if (Area->x < MOUSE_GRAB_PADDING)
			{
				return SDL_HITTEST_RESIZE_TOPLEFT;
			}
			else if (Area->x > Width - MOUSE_GRAB_PADDING)
			{
				return SDL_HITTEST_RESIZE_TOPRIGHT;
			}
			else
			{
				return SDL_HITTEST_RESIZE_TOP;
			}
		}
		else if (Area->y > Height - MOUSE_GRAB_PADDING)
		{
			if (Area->x < MOUSE_GRAB_PADDING)
			{
				return SDL_HITTEST_RESIZE_BOTTOMLEFT;
			}
			else if (Area->x > Width - MOUSE_GRAB_PADDING)
			{
				return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
			}
			else
			{
				return SDL_HITTEST_RESIZE_BOTTOM;
			}
		}
		else if (Area->x < MOUSE_GRAB_PADDING)
		{
			return SDL_HITTEST_RESIZE_LEFT;
		}
		else if (Area->x > Width - MOUSE_GRAB_PADDING)
		{
			return SDL_HITTEST_RESIZE_RIGHT;
		}
		else if (EditorUI::IsTitleBarHovered() && !UI::HoveredBox)
		{
			return SDL_HITTEST_DRAGGABLE;
		}

		return SDL_HITTEST_NORMAL;
	}
#endif

	SDL_Window* Window = nullptr;
	bool ShouldClose = false;
	
	bool WindowHasFocus()
	{
		return SDL_GetKeyboardFocus() == Window || Stats::Time <= 1;
	}

	void Quit()
	{
		ShouldClose = true;
	}
	void Minimize()
	{
		SDL_MinimizeWindow(Window);
	}
	std::set<ButtonEvent> ButtonEvents;
	Shader* PostProcessShader = nullptr;
	Shader* ShadowShader = nullptr;
#if EDITOR
	EditorUI* EditorUserInterface = nullptr;
#endif

	float Timer::TimeSinceCreation() const
	{
		Uint64 PerfCounterFrequency = SDL_GetPerformanceFrequency();
		Uint64 EndCounter = SDL_GetPerformanceCounter();
		Uint64 counterElapsed = EndCounter - Time;
		float Elapsed = ((float)counterElapsed) / ((float)PerfCounterFrequency);
		return Elapsed;
	}
	Timer::Timer()
	{
		Reset();
	}
	void Timer::Reset()
	{
		Time = SDL_GetPerformanceCounter();
	}
	void SetFullScreen(bool NewFullScreen)
	{
#if EDITOR
		if (NewFullScreen)
		{
			SDL_MaximizeWindow(Window);
		}
		else
		{
			SDL_RestoreWindow(Window);
		}
#else
		if (NewFullScreen) SDL_SetWindowFullscreen(Window, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
		else SDL_SetWindowFullscreen(Window, SDL_WINDOW_OPENGL);
		int w, h;
		SDL_GetWindowSize(Window, &w, &h);
		Graphics::SetWindowResolution(Vector2((float)w, (float)h));
		UIBox::RedrawUI();
#endif
	}
	bool GetFullScreen()
	{
#if EDITOR
		auto flag = SDL_GetWindowFlags(Window);
		auto is_fullscreen = flag & SDL_WINDOW_MAXIMIZED;
#else
		auto flag = SDL_GetWindowFlags(Window);
		auto is_fullscreen = flag & SDL_WINDOW_FULLSCREEN;
#endif
		return is_fullscreen;
	}
	void SetCursorPosition(Vector2 NewPos)
	{
		Vector2 Size = GetWindowSize();
		Vector2 TranslatedPos = Vector2(((NewPos.X + 1) / 2) * Size.X, (((NewPos.Y) + 1) / 2) * Size.Y);
		TranslatedPos.Y = Size.Y - TranslatedPos.Y;
		SDL_WarpMouseInWindow(Window, (int)TranslatedPos.X, (int)TranslatedPos.Y);
	}
	Vector2 GetCursorPosition()
	{
		return GetMousePosition();
	}
	Vector2 GetWindowSize()
	{
		int w, h;
		SDL_GetWindowSize(Window, &w, &h);
		return Vector2((float)w, (float)h);
	}
	float LogicTime = 0, RenderTime = 0, SyncTime = 0;
	std::thread ConsoleThread;
}

namespace Input
{
	extern bool Keys[351];
}

namespace LaunchArgs
{
	void EvaluateLaunchArguments(std::vector<std::string> Arguments);
}

void GLAPIENTRY MessageCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam
)
{
	if (type == GL_DEBUG_TYPE_ERROR
		|| type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR
		|| type == GL_DEBUG_TYPE_PORTABILITY)
	{
		Log::Print(std::string(message)  + " - " + Debugging::EngineStatus);
		SDL_Delay(15);
	}
}

void TickObjects()
{
	for (size_t i = 0; i < Objects::AllObjects.size(); i++)
	{
		Objects::AllObjects.at(i)->Tick();
		Objects::AllObjects.at(i)->TickComponents();
	}
#ifdef ENGINE_CSHARP
	CSharp::RunPerFrameLogic();
#endif
}

void DrawFramebuffer(FramebufferObject* Buffer)
{
	if (!Buffer->FramebufferCamera) return;

#if EDITOR

	if (!Application::WindowHasFocus())
	{
		return;
	}

#endif

	Buffer->FramebufferCamera->Update();

	if ((!Buffer->Renderables.size() && !Buffer->ParticleEmitters.size()) || !Buffer->Active)
	{
		Buffer->GetBuffer()->Bind();
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		return;
	}

	if (Buffer->PreviousReflectionCubemapName != Buffer->ReflectionCubemapName)
	{
		Buffer->PreviousReflectionCubemapName = Buffer->ReflectionCubemapName;
		if (Buffer->ReflectionCubemap)
		{
			Cubemap::UnloadCubemapFile(Buffer->ReflectionCubemap);
		}
		if (!Buffer->ReflectionCubemapName.empty())
		{
			Buffer->ReflectionCubemap = Cubemap::LoadCubemapFile(Buffer->ReflectionCubemapName);
		}
		else
		{
			Buffer->ReflectionCubemap = 0;
		}
	}

	for (auto* p : Buffer->ParticleEmitters)
	{
		p->Update(Buffer->FramebufferCamera);
	}
	Debugging::EngineStatus = "Rendering (Framebuffer: Shadows)";

	FrustumCulling::Active = false;
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.f, 0.f, 0.f, 1.f);		//Clear color black
	glViewport(0, 0, Graphics::ShadowResolution, Graphics::ShadowResolution);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	const auto LightSpaceMatrices = CSM::getLightSpaceMatrices();

	CSM::UpdateMatricesUBO();
	if (Graphics::RenderShadows && Graphics::ShadowResolution > 0)
	{
		Graphics::IsRenderingShadows = true;
		glBindFramebuffer(GL_FRAMEBUFFER, CSM::LightFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		for (int j = 0; j < Buffer->Renderables.size(); j++)
		{
			if (Buffer->Renderables[j]->CastShadow)
				Buffer->Renderables.at(j)->SimpleRender(Application::ShadowShader);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		Graphics::IsRenderingShadows = false;
	}

	FrustumCulling::Active = true;
	FrustumCulling::CurrentCameraFrustum = FrustumCulling::createFrustumFromCamera(*Buffer->FramebufferCamera);
	Buffer->GetBuffer()->Bind();
	Debugging::EngineStatus = "Rendering (Framebuffer: Main pass)";

	Vector2 BufferResolution = Buffer->UseMainWindowResolution ? Graphics::WindowResolution : Buffer->CustomFramebufferResolution;
	glViewport(0, 0, (int)BufferResolution.X, (int)BufferResolution.Y);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	auto Matrices = CSM::getLightSpaceMatrices();
	for (auto& s : Shaders)
	{
		Renderable::ApplyDefaultUniformsToShader(s.second.UsedShader, Buffer == Graphics::MainFramebuffer);
		CSM::BindLightSpaceMatricesToShader(Matrices, s.second.UsedShader);
		for (int i = 0; i < 8; i++)
		{
			std::string CurrentLight = "u_lights[" + std::to_string(i) + "]";
			if (i < Buffer->Lights.size())
			{
				s.second.UsedShader->SetVector3(CurrentLight + ".Position", Buffer->Lights[i].Position);
				s.second.UsedShader->SetVector3(CurrentLight + ".Color", Buffer->Lights[i].Color);
				s.second.UsedShader->SetFloat(CurrentLight + ".Falloff", Buffer->Lights[i].Falloff);
				s.second.UsedShader->SetFloat(CurrentLight + ".Intensity", Buffer->Lights[i].Intensity);

				s.second.UsedShader->SetInt(CurrentLight + ".Active", 1);
			}
			else
			{
				s.second.UsedShader->SetInt(CurrentLight + ".Active", 0);
			}
		}
	}
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	if (Graphics::IsWireframe)
	{
		glLineWidth(3);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	// Bind cubemap texture
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Buffer->ReflectionCubemap);
	// Main pass
	BakedLighting::BindToTexture();
	for (auto o : Buffer->Renderables)
	{
		o->Render(Buffer->FramebufferCamera, Buffer == Graphics::MainFramebuffer, false);
	}
	Buffer->GetBuffer()->Bind();
	// Transparency pass
	for (auto p : Buffer->ParticleEmitters)
	{
		p->Draw(Buffer->FramebufferCamera, Buffer == Graphics::MainFramebuffer, true);
	}
	Buffer->GetBuffer()->Bind();
	for (auto o : Buffer->Renderables)
	{
		o->Render(Buffer->FramebufferCamera, Buffer == Graphics::MainFramebuffer, true);
	}


	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, (int)Graphics::WindowResolution.X, (int)Graphics::WindowResolution.Y);
}

void InitializeShaders()
{
	std::cout << "- Initializing Shaders";
	Graphics::TextShader = new Shader("Shaders/UI/text.vert", "Shaders/UI/text.frag");
	std::cout << ".";
	Graphics::UIShader = new Shader("Shaders/UI/uishader.vert", "Shaders/UI/uishader.frag");
	std::cout << ".";
	Application::PostProcessShader = new Shader("Shaders/Internal/postprocess.vert", "Shaders/Internal/postprocess.frag");
	std::cout << ".";
	Application::ShadowShader = new Shader("Shaders/Internal/shadow.vert", "Shaders/Internal/shadow.frag", "Shaders/Internal/shadow.geom");
	Graphics::ShadowShader = Application::ShadowShader;
	std::cout << "." << std::endl;
}


void PollInput()
{
	Input::MouseMovement = Vector2();
	SDL_Event Event;
	while (SDL_PollEvent(&Event))
	{
		if (Event.type == SDL_QUIT)
		{
#if EDITOR
			Application::EditorUserInterface->OnLeave(Application::Quit);
#else
			Application::Quit();
#endif
		}
		else if (Event.type == SDL_MOUSEMOTION)
		{
			Input::MouseMovement += Vector2(Event.motion.xrel / 12.f, -Event.motion.yrel / 12.f);
		}
		else if (Event.type == SDL_KEYDOWN)
		{
			if (Event.key.keysym.sym < 128)
				Input::Keys[Event.key.keysym.sym] = true;
			else
			{
				int sym = Event.key.keysym.sym;
				sym -= 1073741755;
				if (sym > 0)
					Input::Keys[sym] = true;
			}
			switch (Event.key.keysym.sym)
			{
			case SDLK_LEFT:
				TextInput::TextIndex = std::max(std::min(TextInput::TextIndex - 1, (int)TextInput::Text.size()), 0);
				break;
			case  SDLK_RIGHT:
				TextInput::TextIndex = std::max(std::min(TextInput::TextIndex + 1, (int)TextInput::Text.size()), 0);
				break;
			case SDLK_BACKSPACE:
				if (TextInput::PollForText && TextInput::Text.length() > 0)
				{
					if (TextInput::TextIndex == TextInput::Text.size())
					{
						TextInput::Text.pop_back();
					}
					else if (TextInput::TextIndex > 0)
					{
						TextInput::Text.erase((size_t)TextInput::TextIndex - 1, 1);
					}
					TextInput::TextIndex = std::max(std::min(TextInput::TextIndex - 1, (int)TextInput::Text.size()), 0);
				}
				break;
			case SDLK_DELETE:
				if (TextInput::PollForText)
				{
					if (TextInput::TextIndex < TextInput::Text.size() && TextInput::TextIndex >= 0)
					{
						TextInput::Text.erase(TextInput::TextIndex, 1);
					}
				}
				break;
			case SDLK_ESCAPE:
				TextInput::PollForText = false;
				break;
			case SDLK_RETURN:
				TextInput::PollForText = false;
				break;
			case SDLK_F11:
				Application::SetFullScreen(!Application::GetFullScreen());
				break;
			case SDLK_v:
				if (TextInput::PollForText && (Input::IsKeyDown(SDLK_LCTRL) || Input::IsKeyDown(SDLK_RCTRL)))
				{
					std::string ClipboardText = SDL_GetClipboardText();
					if (TextInput::TextIndex < TextInput::Text.size())
					{
						TextInput::Text.insert(TextInput::TextIndex, ClipboardText);
					}
					else
					{
						TextInput::Text.append(ClipboardText);
					}
					TextInput::TextIndex += (int)ClipboardText.size();
				}
				break;
			}
		}
		else if (Event.type == SDL_KEYUP)
		{
			std::vector<int> Indices;

			if (Event.key.keysym.sym < 128)
				Input::Keys[Event.key.keysym.sym] = false;
			else
			{
				int sym = Event.key.keysym.sym;
				sym -= 1073741755;
				if (sym > 0)
					Input::Keys[sym] = false;
			}
		}
		else if (Event.type == SDL_WINDOWEVENT)
		{
			if (Event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				int w, h;
				SDL_GetWindowSize(Application::Window, &w, &h);
				Graphics::SetWindowResolution(Vector2((float)w, (float)h));
				UIBox::RedrawUI();
			}
		}
		else if (Event.type == SDL_MOUSEBUTTONDOWN)
		{
			switch (Event.button.button)
			{
			case SDL_BUTTON_RIGHT:
				Input::IsRMBDown = true;
				TextInput::PollForText = false;
				break;
			case SDL_BUTTON_LEFT:
				Input::IsLMBDown = true;
				break;
			}
		}
		else if (Event.type == SDL_MOUSEBUTTONUP)
		{
			switch (Event.button.button)
			{
			case SDL_BUTTON_RIGHT:
				Input::IsRMBDown = false;
				break;
			case SDL_BUTTON_LEFT:
				Input::IsLMBDown = false;
				break;
			}
		}
		else if (Event.type == SDL_TEXTINPUT)
		{
			if (TextInput::PollForText &&
				!(SDL_GetModState() & KMOD_CTRL &&
					(Event.text.text[0] == 'c' || Event.text.text[0] == 'C' || Event.text.text[0] == 'v' || Event.text.text[0] == 'V')))
			{
				if (Event.text.text[0] >= 32 && Event.text.text[0] <= 128)
				{
					if (TextInput::Text.size() < TextInput::TextIndex)
					{
						TextInput::TextIndex = (int)TextInput::Text.size();
					}
					TextInput::Text.insert(TextInput::TextIndex, std::string(Event.text.text));
					TextInput::TextIndex += (int)strlen(Event.text.text);
				}
			}
		}
		else if (Event.type == SDL_MOUSEWHEEL)
		{
			Sint32 ScrollDistance = Event.wheel.y;
			while (ScrollDistance)
			{
				for (ScrollObject* s : Graphics::UI::ScrollObjects)
				{
					if (Event.wheel.y < 0)
						s->ScrollUp();
					else
						s->ScrollDown();
				}
				if (ScrollDistance < 0)
					ScrollDistance++;
				else
					ScrollDistance--;
			}
		}
	}
	if (Input::CursorVisible)
	{
		Input::MouseLocation = GetMousePosition();
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}
	else
		SDL_SetRelativeMouseMode(SDL_TRUE);
}

void DrawPostProcessing()
{
	bool ShouldSkip3D = false;
#if EDITOR
	if (!Application::WindowHasFocus())
	{
		ShouldSkip3D = true;
	}
#endif


	glViewport(0, 0, (int)Graphics::WindowResolution.X, (int)Graphics::WindowResolution.Y);
	Application::UIMergeEffect->EffectShader->Bind();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, UIBox::GetUITextures()[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, UIBox::GetHighResUITextures()[0]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, UIBox::GetHighResUITextures()[1]);
	Application::UIMergeEffect->EffectShader->SetInt("u_alpha", 1);
	Application::UIMergeEffect->EffectShader->SetInt("u_hr", 2);
	Application::UIMergeEffect->EffectShader->SetInt("u_hrAlpha", 3);
	unsigned int UIBuffer = Application::UIMergeEffect->Render(UIBox::GetUITextures()[0]);

#if !EDITOR
	for (const PostProcess::Effect* i : PostProcess::GetCurrentEffects())
	{
		if (i->UsedType == PostProcess::EffectType::UI)
		{
			UIBuffer = i->Render(UIBuffer);
		}
	}
#endif

	if (!ShouldSkip3D)
	{

		Application::AOBuffer = SSAO::Render(
			Graphics::MainFramebuffer->GetBuffer()->GetTextureID(2),
			Graphics::MainFramebuffer->GetBuffer()->GetTextureID(3));

		Application::MainPostProcessBuffer = Graphics::MainFramebuffer->GetBuffer()->GetTextureID(0);

		if (Application::RenderPostProcess)
		{
			for (const PostProcess::Effect* i : PostProcess::GetCurrentEffects())
			{
				if (i->UsedType == PostProcess::EffectType::World)
				{
					Application::MainPostProcessBuffer = i->Render(Application::MainPostProcessBuffer);
				}
			}
		}
		Application::BloomBuffer = Bloom::BlurFramebuffer(Application::MainPostProcessBuffer);
	}

	Vector2 ActualRes = Application::GetWindowSize();
	glViewport(0, 0, (int)ActualRes.X, (int)ActualRes.Y);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Debugging::EngineStatus = "Rendering (Post process: Main)";

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, Application::MainPostProcessBuffer);
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
	glBindTexture(GL_TEXTURE_2D, Application::BloomBuffer);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, Application::AOBuffer);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, Graphics::MainFramebuffer->GetBuffer()->GetTextureID(1));

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, UIBuffer);
	Application::PostProcessShader->Bind();
	Application::PostProcessShader->SetFloat("u_gamma", Graphics::Gamma);
#if EDITOR
	const auto ViewportTab = Application::EditorUserInterface->UIElements[4];
	const Vector2 ViewportPos = (ViewportTab->Position + ViewportTab->Scale * 0.5);
	Application::PostProcessShader->SetVector2("Position", ViewportPos);
	const Vector2 ViewportScale = ViewportTab->Scale;
	Application::PostProcessShader->SetVector2("Scale", ViewportScale);
#endif
	Application::PostProcessShader->SetInt("u_texture", 1);
	Application::PostProcessShader->SetInt("u_outlines", 2);
	Application::PostProcessShader->SetInt("u_enginearrows", 3);
	Application::PostProcessShader->SetInt("u_ssaotexture", 5);
	Application::PostProcessShader->SetInt("u_depth", 6);
	Application::PostProcessShader->SetInt("u_hasUITexCoords", 1);
	Application::PostProcessShader->SetInt("u_ui", 7);
	Application::PostProcessShader->SetInt("u_uialpha", 8);
	Application::PostProcessShader->SetFloat("u_time", Stats::Time);
	Application::PostProcessShader->SetFloat("u_vignette", Graphics::Vignette);
	Application::PostProcessShader->SetInt("u_fxaa", Graphics::FXAA);
	Application::PostProcessShader->SetInt("u_bloom", Graphics::Bloom);
	Application::PostProcessShader->SetInt("u_ssao", Graphics::SSAO);
	Application::PostProcessShader->SetInt("u_editor", IS_IN_EDITOR);
	Application::PostProcessShader->SetInt("u_hasWindowBorder", IS_IN_EDITOR && !Application::GetFullScreen());
	Application::PostProcessShader->SetVector3("u_borderColor", Application::WindowHasFocus() ? Vector3(0.5, 0.5, 1) : 0.5);
	if (Graphics::Bloom)
	{
		glUniform1i(glGetUniformLocation(Application::PostProcessShader->GetShaderID(), "u_bloomtexture"), 4);
	}
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	Application::PostProcessShader->Unbind();
	glViewport(0, 0, (int)Graphics::WindowResolution.X, (int)Graphics::WindowResolution.Y);
}

void ApplicationLoop()
{
	const Application::Timer FrameTimer;
	const Application::Timer LogicTimer;
	PollInput();
	CameraShake::Tick();
	Sound::Update();

	WorldObject::DestroyMarkedObjects();
	TickObjects();
	float LogicTime = LogicTimer.TimeSinceCreation();
	if (Graphics::MainFramebuffer)
	{
		Graphics::MainCamera = Graphics::MainFramebuffer->FramebufferCamera;
	}
	const Application::Timer RenderTimer;
	Debugging::EngineStatus = "Rendering (Framebuffer)";
	for (FramebufferObject* Buffer : Graphics::AllFramebuffers)
	{
		DrawFramebuffer(Buffer);
	}

	Debugging::EngineStatus = "Responding to button events";
	for (ButtonEvent b : Application::ButtonEvents)
	{
		if (b.c)
		{
			if (b.IsDraggedEvent)
			{
				b.c->OnButtonDragged(b.Index);
			}
			else
			{
				b.c->OnButtonClicked(b.Index);
			}
		}
		if (b.o)
		{
			b.o->OnChildClicked(b.Index);
		}
	}
	Application::ButtonEvents.clear();

	Debugging::EngineStatus = "Ticking (UI)";
	for (int i = 0; i < Graphics::UIToRender.size(); i++)
	{
		Graphics::UIToRender[i]->Tick();
	}

	Debugging::EngineStatus = "Rendering (UI)";
	UIBox::DrawAllUIElements();
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	DrawPostProcessing();
	float RenderTime = RenderTimer.TimeSinceCreation();
#if !EDITOR && !RELEASE
	if (!DebugUI::CurrentDebugUI)
	{
		new DebugUI();
	}
#endif
	Scene::Tick();
	const Application::Timer SwapTimer;
	SDL_GL_SetSwapInterval(0 - Graphics::VSync);
	SDL_GL_SwapWindow(Application::Window);

	Application::RenderTime = RenderTime;
	Application::LogicTime = LogicTime;
	Application::SyncTime = SwapTimer.TimeSinceCreation();

	Performance::DeltaTime = FrameTimer.TimeSinceCreation();
	Performance::DeltaTime *= Performance::TimeMultiplier;
	Performance::FPS = 1 / Performance::DeltaTime;
	Performance::DeltaTime = std::min(Performance::DeltaTime, 0.1f);
	Stats::Time += Performance::DeltaTime;
	Performance::DrawCalls = 0u;

	if (IS_IN_EDITOR && !Application::WindowHasFocus())
	{
		SDL_Delay(100 - (Uint32)(Performance::DeltaTime * 1000));
	}
}

int Initialize(int argc, char** argv)
{
	OS::SetConsoleWindowVisible(true);
	Assets::ScanForAssets();

	for (int i = 0; i < 322; i++) // Array of keys, should be 'false' in the beginning
	{
		Input::Keys[i] = false;
	}

	std::cout << "Starting..." << std::endl;
	std::cout << "- Starting SDL2 - ";
	int SDLReturnValue = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	Application::Timer StartupTimer;
	if (SDLReturnValue != 0)
	{
		std::cout << "Could not start SDL2 (" << SDL_GetError() << ")\n";
	}
	else
	{
		std::cout << "SDL2 started (No error)\n";
	}
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	int flags = SDL_WINDOW_OPENGL;


	// Set Window resolution to the screens resolution * 0.75
	SDL_DisplayMode DM;
	SDL_GetCurrentDisplayMode(0, &DM);
	Graphics::WindowResolution = Vector2((float)DM.w, (float)DM.h) / 1.5f;

	std::string ApplicationTitle = Project::ProjectName;
	if (IsInEditor)
	{
		ApplicationTitle.append(" Editor, v" + std::string(VERSION_STRING));
	}
#if ENGINE_CSHARP && !RELEASE
	if (CSharp::GetUseCSharp())
	{
		ApplicationTitle.append(" (C#)");
	}
#endif

	if (EngineDebug && !IsInEditor)
	{
		ApplicationTitle.append(" (Debug)");
	}
	Application::Window = SDL_CreateWindow(ApplicationTitle.c_str(),
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		(int)Graphics::WindowResolution.X, (int)Graphics::WindowResolution.Y,
		flags);

	SDL_GL_CreateContext(Application::Window);
	SDL_SetWindowResizable(Application::Window, SDL_TRUE);
	

	std::cout << "- Starting GLEW - ";
	if (glewInit() != GLEW_OK)
	{
		std::cout << "GLEW Init Error:\n" << glewGetErrorString(glewInit());
		SDL_DestroyWindow(Application::Window);
		std::cout << "\n-Press Enter to continue-";
		std::cin.get();
		return 1;
	}
	if (!glewIsSupported(OPENGL_MIN_REQUIRED_VERSION))
	{
		SDL_DestroyWindow(Application::Window);
		std::cout << std::string("OpenGL version ")
			+ std::string((const char*)glGetString(GL_VERSION))
			+ std::string(" is not supported. Minimum: ") + OPENGL_MIN_REQUIRED_VERSION << std::endl;
		std::cout << "Press enter to continue";
		std::cin.get();
		std::cout << std::endl;
		return 1;
	}
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	std::cout << "GLEW started (No error)\n";

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Sound::Init();

	Graphics::MainCamera = Scene::DefaultCamera;
	Graphics::MainFramebuffer = new FramebufferObject();
	Graphics::MainFramebuffer->FramebufferCamera = Graphics::MainCamera;

#if ENGINE_CSHARP
	CSharp::Init();
#endif
	BakedLighting::Init();
	Console::InitializeConsole();
	Console::RegisterConVar(Console::Variable("post_process", Type::Bool, &Application::RenderPostProcess, nullptr));
	Cubemap::RegisterCommands();
	InitializeShaders();
	UIBox::InitUI();
	Application::UIMergeEffect = new PostProcess::Effect("Internal/uimerge.frag", PostProcess::EffectType::UI_Internal);
	PostProcess::AddEffect(Application::UIMergeEffect);
	CSM::Init();
	Bloom::Init();
	SSAO::Init();
	if (argc > 1)
	{
		std::vector<std::string> LaunchArguments;
		for (size_t i = 1; i < argc; i++)
		{
			LaunchArguments.push_back(argv[i]);
		}
		LaunchArgs::EvaluateLaunchArguments(LaunchArguments);
	}


	std::string Startup = Project::GetStartupScene();
	if (!Application::StartupSceneOverride.empty())
	{
		Startup = Application::StartupSceneOverride;
	}
	Scene::LoadNewScene(Startup);
	Scene::Tick();
	Project::OnLaunch();

#if EDITOR
	// Initialize EditorUI
	Application::EditorUserInterface = new EditorUI();
#endif

	Log::Print("Finished loading. (" + std::to_string(StartupTimer.TimeSinceCreation()) + " seconds)", Vector3(1.f, 0.75, 0.f));
	if (Application::ShowStartupInfo)
	{
		Console::ExecuteConsoleCommand("info");
	}
	if (!ENGINE_DEBUG && !IS_IN_EDITOR)
	{
		OS::SetConsoleWindowVisible(false);
	}
	while (!Application::ShouldClose)
	{
		ApplicationLoop();
	}
	return 0;
}