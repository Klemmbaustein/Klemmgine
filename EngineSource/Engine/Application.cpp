// Engine includes
#include <Engine/Application.h>
#include <Engine/OS.h>
#include <Engine/Input.h>
#include <Engine/EngineProperties.h>
#include <Engine/Log.h>
#include <Engine/Scene.h>
#include <Engine/Console.h>
#include <Engine/Script.h>

#include <Sound/Sound.h>

#include <UI/Default/UICanvas.h>
#include <UI/UIBox.h>
#include <UI/Default/TextRenderer.h>
#include <UI/EditorUI/EditorUI.h>

#include <Rendering/Renderable.h>
#include <Rendering/Shader.h>
#include <Rendering/Particle.h>
#include <Rendering/Camera/CameraShake.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Rendering/Utility/CSM.h>
#include <Rendering/Utility/SSAO.h>
#include <Rendering/Utility/Bloom.h>
#include <Rendering/Texture/Cubemap.h>
#include <Rendering/Camera/Camera.h>
#include <Rendering/Camera/FrustumCulling.h>

#include <Math/Collision/Collision.h>


// World includes
#include <World/Graphics.h>
#include <World/Assets.h>
#include <World/Stats.h>


// Library includes
#include <GL/glew.h>
#include <SDL.h>


// STL includes
#include <iostream>
#include <thread>

namespace Application
{
	SDL_Window* Window = nullptr;
	bool ShouldClose = false;
	void Quit()
	{
		ShouldClose = true;
	}
	std::set<ButtonEvent> ButtonEvents;
	Shader* PostProcessShader = nullptr;
	Shader* ShadowShader = nullptr;
	TextRenderer* DebugTextRenderer = nullptr;
	bool IsWindowFullscreen = false;
#if EDITOR
	EditorUI* EditorUserInterface = nullptr;
#endif

	float Timer::TimeSinceCreation()
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
		IsWindowFullscreen = NewFullScreen;
		if (IsInEditor)
		{
			if (Application::IsWindowFullscreen)
				SDL_MaximizeWindow(Application::Window);
			else
				SDL_RestoreWindow(Application::Window);
		}
		else
		{
			if (Application::IsWindowFullscreen) SDL_SetWindowFullscreen(Application::Window, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
			else SDL_SetWindowFullscreen(Application::Window, SDL_WINDOW_OPENGL);
			int w, h;
			SDL_GetWindowSize(Application::Window, &w, &h);
			Graphics::SetWindowResolution(Vector2(w, h));
			UIBox::RedrawUI();
		}
	}
	bool GetFullScreen()
	{
		return IsWindowFullscreen;
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

std::string GetStartupScene();

void GLAPIENTRY
MessageCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam
)
{
	if ((type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR || type == GL_DEBUG_TYPE_PORTABILITY))
	{
		Log::Print(message + std::string(" Status: ") + Debugging::EngineStatus);
	}
}

void TickObjects()
{
	for (size_t i = 0; i < Objects::AllObjects.size(); i++)
	{
		Objects::AllObjects.at(i)->Tick();
		Objects::AllObjects.at(i)->TickComponents();
	}
}

std::string GetConsoleInput()
{
	std::string in;
	std::cin >> in;
	return in;
}


void DrawFramebuffer(FramebufferObject* Buffer)
{
	if (Buffer->FramebufferCamera)
	{
		Buffer->FramebufferCamera->Update();
	}
	else return;

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
	if (Graphics::RenderShadows)
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

	Debugging::EngineStatus = "Rendering (Framebuffer: Main pass)";
	FrustumCulling::Active = true;
	FrustumCulling::CurrentCameraFrustum = FrustumCulling::createFrustumFromCamera(*Buffer->FramebufferCamera);
	Buffer->GetBuffer()->Bind();
	glViewport(0, 0, Graphics::WindowResolution.X, Graphics::WindowResolution.Y);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	auto Matrices = CSM::getLightSpaceMatrices();
	for (const auto& s : Shaders)
	{
		Renderable::ApplyDefaultUniformsToShader(s.second.UsedShader);
		CSM::BindLightSpaceMatricesToShader(Matrices, s.second.UsedShader);
		for (int i = 0; i < 8; i++)
		{
			std::string CurrentLight = "u_lights[" + std::to_string(i) + "]";
			if (i < Graphics::Lights.size())
			{
				glUniform3fv(glGetUniformLocation(s.second.UsedShader->GetShaderID(), (CurrentLight + ".Position").c_str()), 1, &Graphics::Lights[i].Position.X);
				glUniform3fv(glGetUniformLocation(s.second.UsedShader->GetShaderID(), (CurrentLight + ".Color").c_str()), 1, &Graphics::Lights[i].Color.X);
				glUniform1f(glGetUniformLocation(s.second.UsedShader->GetShaderID(), (CurrentLight + ".Falloff").c_str()), Graphics::Lights[i].Falloff);
				glUniform1f(glGetUniformLocation(s.second.UsedShader->GetShaderID(), (CurrentLight + ".Intensity").c_str()), Graphics::Lights[i].Intensity);

				glUniform1i(glGetUniformLocation(s.second.UsedShader->GetShaderID(), (CurrentLight + ".Active").c_str()), 1);
			}
			else
			{
				glUniform1i(glGetUniformLocation(s.second.UsedShader->GetShaderID(), (CurrentLight + ".Active").c_str()), 0);
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
	for (auto o : Buffer->Renderables)
	{
		o->Render(Buffer->FramebufferCamera, Buffer == Graphics::MainFramebuffer, false);
	}
	Buffer->GetBuffer()->Bind();
	for (auto p : Buffer->ParticleEmitters)
	{
		p->Draw(Buffer->FramebufferCamera, Buffer == Graphics::MainFramebuffer, false);
	}
	// Transparency pass
	for (auto o : Buffer->Renderables)
	{
		o->Render(Buffer->FramebufferCamera, Buffer == Graphics::MainFramebuffer, true);
	}
	Buffer->GetBuffer()->Bind();
	for (auto p : Buffer->ParticleEmitters)
	{
		p->Draw(Buffer->FramebufferCamera, Buffer == Graphics::MainFramebuffer, true);
	}


	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void InitializeShaders()
{
	std::cout << "Initializing Shaders";
	Graphics::TextShader = new Shader("Shaders/text.vert", "Shaders/text.frag");
	std::cout << ".";
	Graphics::UIShader = new Shader("Shaders/uishader.vert", "Shaders/uishader.frag");
	std::cout << ".";
	Application::PostProcessShader = new Shader("Shaders/postprocess.vert", "Shaders/postprocess.frag");
	std::cout << ".";
	Application::ShadowShader = new Shader("Shaders/shadow.vert", "Shaders/shadow.frag", "Shaders/shadow.geom");
	Graphics::ShadowShader = Application::ShadowShader;
	std::cout << "." << std::endl;
}

Vector2 GetMousePosition()
{
	int x;
	int y;
	SDL_GetMouseState(&x, &y);
	return Vector2((x / Graphics::WindowResolution.X - 0.5f) * 2, 1 - (y / Graphics::WindowResolution.Y * 2));
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
			std::vector<int> Indices;

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
						TextInput::Text.erase(TextInput::TextIndex - 1, 1);
					}
					TextInput::TextIndex = std::max(std::min(TextInput::TextIndex - 1, (int)TextInput::Text.size()), 0);
				}
				break;
			case SDLK_DELETE:
				if (!TextInput::PollForText)
					for (int i = 0; i < Objects::AllObjects.size(); i++)
					{
						if (Objects::AllObjects.at(i)->IsSelected)
						{
							Objects::DestroyObject(Objects::AllObjects[i]);

						}
					}
#if IS_IN_EDITOR
				//Application::EditorUserInterface->UpdateObjectList();
				break;
#endif
			case SDLK_ESCAPE:
				for (int i = 0; i < Objects::AllObjects.size(); i++)
				{
					Objects::AllObjects.at(i)->IsSelected = false;
				}
				TextInput::PollForText = false;
				break;
			case SDLK_RETURN:
				TextInput::PollForText = false;
				break;
			case SDLK_F11:
				Application::SetFullScreen(!Application::IsWindowFullscreen);
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
				Graphics::SetWindowResolution(Vector2(w, h));
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
					TextInput::Text.insert(TextInput::TextIndex, std::string(Event.text.text));
					TextInput::TextIndex += strlen(Event.text.text);
				}
			}
		}
		else if (Event.type == SDL_MOUSEWHEEL)
		{
			for (ScrollObject* s : Graphics::UI::ScrollObjects)
			{
				if (Event.wheel.y < 0)
					s->ScrollUp();
				else
					s->ScrollDown();
			}
		}
	}
	Input::MouseLocation = GetMousePosition();
	if (!IsInEditor)
	{
		if (Input::CursorVisible)
			SDL_SetRelativeMouseMode(SDL_FALSE);
		else
			SDL_SetRelativeMouseMode(SDL_TRUE);
	}
}

void DrawPostProcessing()
{
	unsigned int BloomTexture = Bloom::BlurFramebuffer(Graphics::MainFramebuffer->GetTextureID());

	unsigned int SSAOTexture = SSAO::Render(
		Graphics::MainFramebuffer->GetBuffer()->GetTextureID(2),
		Graphics::MainFramebuffer->GetBuffer()->GetTextureID(3));

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Graphics::WindowResolution.X, Graphics::WindowResolution.Y);
	Debugging::EngineStatus = "Rendering (Post process: Main)";
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, Graphics::MainFramebuffer->GetBuffer()->GetTextureID(0));
#if IS_IN_EDITOR
	glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, Application::EditorUserInterface->OutlineFramebuffer->GetBuffer()->GetTextureID(1));
	glActiveTexture(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_2D, Application::EditorUserInterface->ArrowFramebuffer->GetTextureID());
#endif
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, BloomTexture);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, SSAOTexture);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, Graphics::MainFramebuffer->GetBuffer()->GetTextureID(1));
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, UIBox::GetUIFramebuffer());
	Application::PostProcessShader->Bind();
	glUniform1f(glGetUniformLocation(Application::PostProcessShader->GetShaderID(), "u_gamma"), Graphics::Gamma);
	glUniform1i(glGetUniformLocation(Application::PostProcessShader->GetShaderID(), "FullScreen"), !IsInEditor);
	glUniform1i(glGetUniformLocation(Application::PostProcessShader->GetShaderID(), "u_texture"), 1);
	glUniform1i(glGetUniformLocation(Application::PostProcessShader->GetShaderID(), "u_outlines"), 2);
	glUniform1i(glGetUniformLocation(Application::PostProcessShader->GetShaderID(), "u_enginearrows"), 3);
	glUniform1i(glGetUniformLocation(Application::PostProcessShader->GetShaderID(), "u_ssaotexture"), 5);
	glUniform1i(glGetUniformLocation(Application::PostProcessShader->GetShaderID(), "u_depth"), 6);
	glUniform1i(glGetUniformLocation(Application::PostProcessShader->GetShaderID(), "u_ui"), 7);
	glUniform1f(glGetUniformLocation(Application::PostProcessShader->GetShaderID(), "u_time"), Stats::Time);
	glUniform1f(glGetUniformLocation(Application::PostProcessShader->GetShaderID(), "u_vignette"), Graphics::Vignette);
	glUniform1i(glGetUniformLocation(Application::PostProcessShader->GetShaderID(), "u_fxaa"), Graphics::FXAA);
	glUniform1i(glGetUniformLocation(Application::PostProcessShader->GetShaderID(), "u_bloom"), Graphics::Bloom);
	glUniform1i(glGetUniformLocation(Application::PostProcessShader->GetShaderID(), "u_ssao"), Graphics::SSAO);
	if (Graphics::Bloom)
	{
		glUniform1i(glGetUniformLocation(Application::PostProcessShader->GetShaderID(), "u_bloomtexture"), 4);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	Application::PostProcessShader->Unbind();
}

void ApplicationLoop()
{
	Application::Timer FrameTimer;
	Application::Timer LogicTimer;
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
	Application::Timer RenderTimer;
	Debugging::EngineStatus = "Rendering (Framebuffer)";
	for (FramebufferObject* Buffer : Graphics::AllFramebuffers)
	{
		DrawFramebuffer(Buffer);
	}

	Debugging::EngineStatus = "Rendering (UI)";
	UI::NewHoveredButton = nullptr;
	UIBox::DrawAllUIElements();
	UI::HoveredButton = UI::NewHoveredButton;

	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	DrawPostProcessing();
	float RenderTime = RenderTimer.TimeSinceCreation();
#if !IS_IN_EDITOR && ENGINE_DEBUG
	Debugging::EngineStatus = "Rendering (Debug Text)";

	Application::DebugTextRenderer->RenderText({ TextSegment("FPS: " + std::to_string((int)Performance::FPS), Vector3(1, 1, 0)) }, Vector2(-0.95, 0.9),
		2, Vector3(1, 1, 0), 1, 999, nullptr);
	std::string DeltaString = "Delta: " + std::to_string((int)(Performance::DeltaTime * 1000)) + "ms";

	Application::DebugTextRenderer->RenderText({ TextSegment(DeltaString, Vector3(1, 1, 0)) },
		Vector2(-0.95, 0.85), 2, 1, 1, 999, nullptr);
	DeltaString.clear();
	DeltaString.append(std::to_string((int)(Application::LogicTime / Performance::DeltaTime * 100.f)) + "% Log ");
	DeltaString.append(std::to_string((int)(Application::RenderTime / Performance::DeltaTime * 100.f)) + "% Rend ");
	DeltaString.append(std::to_string((int)(Application::SyncTime / Performance::DeltaTime * 100.f)) + "% Buf");
	Application::DebugTextRenderer->RenderText({ TextSegment(DeltaString, Vector3(1, 1, 0)) },
		Vector2(-0.95, 0.8), 2, 1, 1, 999, nullptr);
	Application::DebugTextRenderer->RenderText({ TextSegment("DrawCalls: " + std::to_string(Performance::DrawCalls), Vector3(1, 1, 0)) },
		Vector2(-0.95, 0.75), 2, 1, 1, 999, nullptr);
	Application::DebugTextRenderer->RenderText({ TextSegment("CollisonMeshes: " + std::to_string(Collision::CollisionBoxes.size()), Vector3(1, 1, 0)) },
		Vector2(-0.95, 0.7), 2, Vector3(1, 1, 0), 1, 999, nullptr);
#endif
	Debugging::EngineStatus = "Ticking (UI)";
	for (int i = 0; i < Graphics::UIToRender.size(); i++)
	{
		Graphics::UIToRender[i]->Tick();
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
	Scene::Tick();
	Application::Timer SwapTimer;
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
	std::cout << "*Starting SDL2 - ";
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
	Graphics::WindowResolution = Vector2(DM.w, DM.h) / 1.5;

	std::string ApplicationTitle = ProjectName;
	if (IsInEditor) ApplicationTitle.append(" Editor, v" + std::string(VERSION_STRING));
	if (EngineDebug && !IsInEditor) ApplicationTitle.append(" (Debug)");
	Application::Window = SDL_CreateWindow(ApplicationTitle.c_str(),
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		Graphics::WindowResolution.X, Graphics::WindowResolution.Y,
		flags);

	SDL_GL_CreateContext(Application::Window);
	SDL_SetWindowResizable(Application::Window, SDL_TRUE);

	std::cout << "*Starting GLEW - ";
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

	glEnable(GL_MULTISAMPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Sound::Init();

	Graphics::MainCamera = Scene::DefaultCamera;
	Graphics::MainFramebuffer = new FramebufferObject();
	Graphics::MainFramebuffer->FramebufferCamera = Graphics::MainCamera;

	CSM::Init();
	Bloom::Init();
	SSAO::Init();
	Console::InitializeConsole();
	Cubemap::RegisterCommands();
	InitializeShaders();

	UIBox::InitUI();
	Application::DebugTextRenderer = new TextRenderer("Font.ttf", 60);

	if (argc > 1)
	{
		std::vector<std::string> LaunchArguments;
		for (size_t i = 1; i < argc; i++)
		{
			LaunchArguments.push_back(argv[i]);
		}
		LaunchArgs::EvaluateLaunchArguments(LaunchArguments);
	}
	Scene::LoadNewScene(GetStartupScene());
	Scene::Tick();
#if EDITOR
	// Initialize EditorUI
	Application::EditorUserInterface = new EditorUI();
	Graphics::UIToRender.push_back(Application::EditorUserInterface);

	Config::LoadConfigs();
#endif
	Log::Print(std::string("Finished loading. (").append(std::to_string(StartupTimer.TimeSinceCreation()).append(" seconds)")), Vector3(1.f, 0.75, 0.f));
	Console::ExecuteConsoleCommand("info");
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