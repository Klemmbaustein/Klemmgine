// Engine includes
#define SDL_MAIN_HANDLED
#include <Engine/Application.h>
#include <Engine/OS.h>
#include <Engine/Input.h>
#include <Engine/EngineProperties.h>
#include <Engine/Log.h>
#include <Engine/EngineError.h>
#include <Engine/File/Assets.h>
#include <Engine/Stats.h>
#include <Engine/Gamepad.h>

#include <Engine/Subsystem/Console.h>
#include <Engine/Subsystem/Sound.h>
#include <Engine/Subsystem/CSharpInterop.h>
#include <Engine/Subsystem/NetworkSubsystem.h>
#include <Engine/Subsystem/InputSubsystem.h>
#include <Engine/Subsystem/PhysicsSubsystem.h>
#include <Engine/Subsystem/LogSubsystem.h>
#include <Engine/Subsystem/BackgroundTask.h>
#include <Engine/Subsystem/Scene.h>

#include <UI/UIBox.h>
#include <UI/EditorUI/EditorUI.h>
#include <UI/Debug/DebugUI.h>

#include <Rendering/Camera/CameraShake.h>
#include <Rendering/Framebuffer.h>
#include <Rendering/Camera/Camera.h>

#include <Math/Collision/CollisionVisualize.h>
#include <Math/Collision/Collision.h>

#include <Networking/Networking.h>

// Library includes
#include <GL/glew.h>
#include <SDL.h>

// STL includes
#include <iostream>
#include <thread>
#include <deque>
#include <cstdint>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <Rendering/RenderSubsystem/PostProcess.h>

static Vector2 GetMousePosition()
{
#if SERVER
	return 0;
#endif
	int x;
	int y;
	SDL_GetMouseState(&x, &y);
	Vector2 Size = Application::GetWindowSize();
	return Vector2((x / Size.X - 0.5f) * 2, 1 - (y / Size.Y * 2));
}

static std::string ToAppTitle(std::string Name)
{
	std::string ApplicationTitle = Name;
#if EDITOR
	ApplicationTitle.append(" Editor, v" + std::string(VERSION_STRING));
#endif
#if ENGINE_CSHARP && !RELEASE
	if (CSharpInterop::GetUseCSharp())
	{
		ApplicationTitle.append(" (C#)");
	}
#endif

	if (EngineDebug && !IsInEditor)
	{
		ApplicationTitle.append(" (Debug)");
	}
	return ApplicationTitle;
}

namespace Application
{
	std::string StartupSceneOverride;
	bool ShowStartupInfo = true;

	SDL_Window* Window = nullptr;
	bool ShouldClose = false;
	
	bool WindowHasFocus()
	{
#if SERVER
		return false;
#endif
		return SDL_GetKeyboardFocus() == Window || Stats::Time <= 1;
	}

	void Quit()
	{
		ShouldClose = true;
	}
	void Minimize()
	{
#if SERVER
		return;
#endif
		SDL_MinimizeWindow(Window);
	}
	std::set<ButtonEvent> ButtonEvents;
#if EDITOR
	EditorUI* EditorInstance = nullptr;
#endif

	float Timer::Get() const
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
#if !SERVER
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
#endif
#endif
	}
	bool GetFullScreen()
	{
#if SERVER
		return false;
#endif
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
#if SERVER
		return;
#endif
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
#if SERVER
		return 0;
#endif
		int w, h;
		SDL_GetWindowSize(Window, &w, &h);
		return Vector2((float)w, (float)h);
	}
	std::string EditorPath;

	void SetEditorPath(std::string NewEditorPath)
	{
		EditorPath = NewEditorPath;
	}

	std::string GetEditorPath()
	{
		return EditorPath;
	}
	float LogicTime = 0, RenderTime = 0, SyncTime = 0;
	size_t FrameCount = 0;
#if !SERVER
#endif
}

namespace LaunchArgs
{
	void EvaluateLaunchArguments(std::vector<std::string> Arguments);
}

static void GLAPIENTRY MessageCallback(
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
		Log::Print(std::string(message)  + " - " + Stats::EngineStatus, Log::LogColor::Red);
		SDL_Delay(5);
	}
}


static void UpdateObjects()
{
	Stats::EngineStatus = "Updating objects";
	WorldObject::DestroyMarkedObjects();
	for (size_t i = 0; i < Objects::AllObjects.size(); i++)
	{
		Objects::AllObjects.at(i)->Update();
		Objects::AllObjects.at(i)->UpdateComponents();
	}
}

static void ApplicationLoop()
{
	const Application::Timer FrameTimer;
	const Application::Timer LogicTimer;
	Subsystem::UpdateSubsystems();
#if !SERVER
	CameraShake::Tick();
#endif
#if !EDITOR
	if (Project::UseNetworkFunctions)
	{
		Networking::Update();
	}
#endif

	UpdateObjects();
	float LogicTime = LogicTimer.Get();
	const Application::Timer RenderTimer;
#if !SERVER
	Stats::EngineStatus = "Rendering (Framebuffer)";
	for (FramebufferObject* Buffer : Graphics::AllFramebuffers)
	{
		Buffer->Draw();
	}
	UIBox::UpdateUI();
	UIBox::DrawAllUIElements();
	float RenderTime = RenderTimer.Get();
#if !EDITOR && !RELEASE
	if (!Debug::DebugUI::CurrentDebugUI)
	{
		new Debug::DebugUI();
	}
#endif
	PostProcess::PostProcessSystem->Draw();
#endif
	const Application::Timer SwapTimer;
#if !SERVER
	SDL_GL_SetSwapInterval(0 - Graphics::VSync);
	SDL_GL_SwapWindow(Application::Window);
#endif

#if !SERVER
	Application::RenderTime = RenderTime;
#endif
	Application::LogicTime = LogicTime;
	Application::SyncTime = SwapTimer.Get();
	Application::FrameCount++;

	Stats::DeltaTime = FrameTimer.Get();
	Stats::DeltaTime *= Stats::TimeMultiplier;
	Stats::FPS = 1 / Stats::DeltaTime;
	Stats::DeltaTime = std::min(Stats::DeltaTime, 0.1f);
	Stats::DrawCalls = 0u;

#if EDITOR
	if (!Application::WindowHasFocus())
	{
		SDL_Delay(100 - (Uint32)(Stats::DeltaTime * 1000));
	}
#endif
#if SERVER
	if (Networking::GetTickRate() > Stats::DeltaTime)
	{
		SDL_Delay((Uint32)(1000.0f
			* 1.0f / (float)Networking::GetTickRate() - Stats::DeltaTime));
	}

	Stats::DeltaTime = std::max(Stats::DeltaTime, 1.0f / (float)Networking::GetTickRate());
#endif
	Stats::Time += Stats::DeltaTime;
}

static void CreateWindow()
{
	std::cout << "Starting..." << std::endl;
	std::cout << "- Starting SDL2 - ";
	int SDLReturnValue = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK);
	if (SDLReturnValue != 0)
	{
		std::cout << "Could not start SDL2 (" << SDL_GetError() << ")\n";
		exit(1);
	}
	else
	{
		std::cout << "SDL2 started (No error)\n";
	}
#if !SERVER
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
	Graphics::RenderResolution = Graphics::WindowResolution;
	Application::Window = SDL_CreateWindow(ToAppTitle(Project::ProjectName).c_str(),
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		(int)Graphics::WindowResolution.X, (int)Graphics::WindowResolution.Y,
		flags);

	SDL_GL_CreateContext(Application::Window);
	SDL_SetWindowResizable(Application::Window, SDL_TRUE);

	std::cout << "- Starting GLEW - ";
	auto GlewStatus = glewContextInit();
	if (GlewStatus != GLEW_OK)
	{
		std::cout << "GLEW Init Error:\n" << glewGetErrorString(GlewStatus);
		SDL_DestroyWindow(Application::Window);
		std::cout << "\nPress Enter to continue";
		std::cin.get();
		exit(1);
	}
	if (!glewIsSupported(OPENGL_MIN_REQUIRED_VERSION))
	{
		SDL_DestroyWindow(Application::Window);
		std::cout << std::string("OpenGL version ");
		std::cout << (const char*)glGetString(GL_VERSION);
		std::cout << std::string(" is not supported. Minimum: ") + OPENGL_MIN_REQUIRED_VERSION << std::endl;
		std::cout << "Press enter to continue";
		std::cin.get();
		std::cout << std::endl;
		exit(1);
	}
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	std::cout << "GLEW started (No error)" << std::endl;
#endif
}

int Application::Initialize(int argc, char** argv)
{
	Application::Timer StartupTimer;
	OS::SetConsoleWindowVisible(true);
	Assets::ScanForAssets();
	Application::EditorPath = std::filesystem::current_path().u8string();

	CreateWindow();

	Error::Init();

	Subsystem::Load(new LogSubsystem());
	Subsystem::Load(new Console());
	Subsystem::Load(new PhysicsSubsystem());
	Subsystem::Load(new BackgroundTaskSubsystem());
	Subsystem::Load(new Scene());
#if !SERVER
	Subsystem::Load(new InputSubsystem());
	Subsystem::Load(new Sound());
#endif
#if !EDITOR
	if (Project::UseNetworkFunctions)
	{
		Subsystem::Load(new NetworkSubsystem());
	}
#endif

	if (argc > 1)
	{
		std::vector<std::string> LaunchArguments;
		for (size_t i = 1; i < argc; i++)
		{
			LaunchArguments.push_back(argv[i]);
		}
		LaunchArgs::EvaluateLaunchArguments(LaunchArguments);
	}
#if ENGINE_CSHARP
	Subsystem::Load(new CSharpInterop());

#if ENGINE_NO_SOURCE
	SDL_SetWindowTitle(Window, ToAppTitle(
		CSharpInterop::StaticCall<const char*>(
			CSharpInterop::CSharpSystem->LoadCSharpFunction("GetNameInternally", "Engine", "StringDelegate")
		)
	).c_str());
#endif
#endif

#if !SERVER
	UIBox::InitUI();

	RenderSubsystem::LoadRenderSubsystems();

	Console::ConsoleSystem->RegisterCommand(Console::Command("show_collision", CollisionVisualize::Activate, {}));
	Console::ConsoleSystem->RegisterCommand(Console::Command("hide_collision", CollisionVisualize::Deactivate, {}));

#endif

	std::string Startup = Project::GetStartupScene();
	if (!Application::StartupSceneOverride.empty())
	{
		Startup = Application::StartupSceneOverride;
	}
	Scene::LoadNewScene(Startup, true);
	Project::OnLaunch();

#if EDITOR
	Subsystem::Load(new EditorUI());
#endif

	Log::Print("Finished loading. (" + std::to_string(StartupTimer.Get()) + " seconds)", Vector3(1.f, 0.75, 0.f));
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
	Subsystem::DestroyAll();
	OS::SetConsoleWindowVisible(true);
	exit(0);
}

int main(int argc, char** argv)
{
	return Application::Initialize(argc, argv);
}