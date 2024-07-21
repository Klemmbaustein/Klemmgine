#define SDL_MAIN_HANDLED
#include <Engine/Application.h>
#include <Engine/OS.h>
#include <Engine/Input.h>
#include <Engine/EngineProperties.h>
#include <Engine/Log.h>
#include <Engine/EngineError.h>
#include <Engine/File/Assets.h>
#include <Engine/Stats.h>
#include <Engine/AppWindow.h>
#include <Engine/LaunchArgs.h>
#include <Engine/Utility/StringUtility.h>

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
#include <Rendering/RenderSubsystem/PostProcess.h>

#include <Math/Collision/CollisionVisualize.h>
#include <Math/Collision/Collision.h>

#include <Networking/Networking.h>

#include <GL/glew.h>
#include <SDL.h>

#include <iostream>
#include <thread>
#include <cstdint>


namespace Application
{
	std::string StartupSceneOverride;
	bool ShowStartupInfo = true;

	bool ShouldClose = false;

	void Quit()
	{
		ShouldClose = true;
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

	std::string EditorPath;

	void SetEditorPath(std::string NewEditorPath)
	{
		EditorPath = NewEditorPath;
	}

	std::string GetEditorPath()
	{
		return EditorPath;
	}
}

static void UpdateObjects()
{
	Stats::EngineStatus = "Updating objects";
	SceneObject::DestroyMarkedObjects(true);
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

#if !EDITOR && !RELEASE
	if (!Debug::DebugUI::CurrentDebugUI)
	{
		new Debug::DebugUI();
	}
#endif

	PostProcess::PostProcessSystem->Draw();
#endif
	float RenderTime = RenderTimer.Get();

	const Application::Timer SwapTimer;
#if !SERVER
	SDL_GL_SetSwapInterval(0 - Graphics::VSync);
	SDL_GL_SwapWindow(Window::SDLWindow);
#endif

#if !SERVER
	Stats::RenderTime = RenderTime;
#endif
	Stats::LogicTime = LogicTime;
	Stats::SyncTime = SwapTimer.Get();
	Stats::FrameCount++;

	Stats::DeltaTime = FrameTimer.Get() * Stats::TimeMultiplier;
	Stats::FPS = 1 / Stats::DeltaTime;
	Stats::DrawCalls = 0u;

#if EDITOR
	// Slow down the editor if it doesn't have focus.
	// This saves performance.
	if (!Window::WindowHasFocus())
	{
		SDL_Delay(100 - (Uint32)(Stats::DeltaTime * 1000));
	}
#endif
#if SERVER

	if (Networking::GetTickDelta() > Stats::DeltaTime)
	{
		// Sleep to maintain a constant update rate on a server
		float SleepDelay = Networking::GetTickDelta() - Stats::DeltaTime;

		std::this_thread::sleep_for(std::chrono::microseconds(Uint32(1000.0f * 1000.0f * SleepDelay)));
	}

	Stats::DeltaTime = std::max(Stats::DeltaTime, Networking::GetTickDelta());
#endif
	Stats::Time += Stats::DeltaTime;
}

static void InitSDL()
{
	std::cout << "Starting..." << std::endl;
	std::cout << "- Starting SDL2 - ";

#if !SERVER
	int SDLReturnValue = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK);
#else
	// A server only needs SDL_INIT_EVENTS.
	// SDL still needs to be initialized for SDL_net.
	int SDLReturnValue = SDL_Init(SDL_INIT_EVENTS);
#endif

	if (SDLReturnValue != 0)
	{
		std::cout << "Could not start SDL2 (" << SDL_GetError() << ")\n";
		exit(1);
	}
	else
	{
		std::cout << "SDL2 started (No error)\n";
	}
}

int Application::Initialize(int argc, char** argv)
{
	Application::Timer StartupTimer;
	OS::SetConsoleWindowVisible(true);
	Assets::ScanForAssets();
	Application::EditorPath = StrUtil::UnicodeToAscii(std::filesystem::current_path().u8string());
	Error::Init();

	InitSDL();
#if !SERVER
	Window::InitWindow(Project::ProjectName);
#endif

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
		Subsystem::Load(new NetworkSubsystem(12345));
	}
#endif

	// Evaluating launch args depends on some subsystems (LogSubsystem, Console, Scene...), so these can't be evaluated before here.
	LaunchArgs::Evaluate(argc, argv);

#if ENGINE_CSHARP
	Subsystem::Load(new CSharpInterop());
#endif

#if !SERVER
	UIBox::InitUI();

	Subsystem::Load(new RenderSubsystem());

#endif

	std::string Startup = Project::GetStartupScene();

	// Application::StartupSceneOverride is set mostly by launch args. (-scene xyz)
	// Some projects might still depend on GetStartupScene always being called, which is why it's alwayys called here.
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
#if !SERVER
	OS::SetConsoleWindowVisible(true);
	Window::DestroyWindow();
#endif

	Subsystem::DestroyAll();
	exit(0);
}

int main(int argc, char** argv)
{
	return Application::Initialize(argc, argv);
}