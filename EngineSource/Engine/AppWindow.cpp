#if !SERVER
#include "AppWindow.h"
#include <SDL.h>
#include "Subsystem/CSharpInterop.h"
#include "EngineProperties.h"
#include "Stats.h"
#include <Rendering/Graphics.h>
#include <iostream>
#include <GL/glew.h>
#include <Engine/EngineError.h>
#if _WIN32
#include <SDL_syswm.h>
#include <dwmapi.h>

#pragma comment(lib, "Dwmapi.lib")
#endif

// TODO: Multiple windows?

SDL_Window* Window::SDLWindow = nullptr;
SDL_GLContext OpenGLContext = nullptr;

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

#if !RELEASE && !EDITOR
	{
		ApplicationTitle.append(" (Debug)");
	}
#endif
	return ApplicationTitle;
}

void Window::SetCursorPosition(Vector2 NewPos)
{
#if SERVER
	return;
#endif
	Vector2 Size = Window::GetWindowSize();
	Vector2 TranslatedPos = Vector2(((NewPos.X + 1) / 2) * Size.X, (((NewPos.Y) + 1) / 2) * Size.Y);
	TranslatedPos.Y = Size.Y - TranslatedPos.Y;
	SDL_WarpMouseInWindow(SDLWindow, (int)TranslatedPos.X, (int)TranslatedPos.Y);
}

Vector2 Window::GetCursorPosition()
{
	int x;
	int y;
	SDL_GetMouseState(&x, &y);
	Vector2 Size = Window::GetWindowSize();
	return Vector2((x / Size.X - 0.5f) * 2, 1 - (y / Size.Y * 2));
}

void Window::SetTitleBarDark(bool NewIsDark)
{
	// Adjust some attributes to make the editor window look prettier
#if _WIN32 && EDITOR
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(SDLWindow, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	BOOL UseDarkMode = NewIsDark;
	DwmSetWindowAttribute(
		hwnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE,
		&UseDarkMode, sizeof(UseDarkMode));
#endif
}

void Window::InitWindow(std::string WindowTitle)
{
	ENGINE_ASSERT(!SDLWindow, "Window::InitWindow has been called but the window already exists.");

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
	SDLWindow = SDL_CreateWindow(ToAppTitle(WindowTitle).c_str(),
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		(int)Graphics::WindowResolution.X, (int)Graphics::WindowResolution.Y,
		flags);

	OpenGLContext = SDL_GL_CreateContext(SDLWindow);
	SDL_SetWindowResizable(SDLWindow, SDL_TRUE);

	std::cout << "- Starting GLEW - ";
	auto GlewStatus = glewContextInit();
	if (GlewStatus != GLEW_OK)
	{
		std::cout << "GLEW Init Error:\n" << glewGetErrorString(GlewStatus);
		SDL_DestroyWindow(SDLWindow);
		std::cout << "\nPress Enter to continue";
		std::cin.get();
		exit(1);
	}
	if (!glewIsSupported(OPENGL_MIN_REQUIRED_VERSION))
	{
		SDL_DestroyWindow(SDLWindow);
		std::cout << std::string("OpenGL version ");
		std::cout << (const char*)glGetString(GL_VERSION);
		std::cout << std::string(" is not supported. Minimum: ") + OPENGL_MIN_REQUIRED_VERSION << std::endl;
		std::cout << "Press enter to continue";
		std::cin.get();
		std::cout << std::endl;
		exit(1);
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	std::cout << "GLEW started (No error)" << std::endl;

	SetTitleBarDark(true);
}

void Window::DestroyWindow()
{
	ENGINE_ASSERT(SDLWindow, "Window::DestroyWindow() has been called but the window doesn't exist.");
	SDL_DestroyWindow(SDLWindow);
	SDL_GL_DeleteContext(OpenGLContext);
}

void Window::SetWindowTitle(std::string NewTitle)
{
	SDL_SetWindowTitle(SDLWindow, ToAppTitle(NewTitle).c_str());
}

void Window::Minimize()
{
	SDL_MinimizeWindow(SDLWindow);
}

bool Window::WindowHasFocus()
{
#if SERVER
	return false;
#endif
	return SDL_GetKeyboardFocus() == SDLWindow || Stats::Time <= 1;
}

void Window::SetFullScreen(bool NewFullScreen)
{
#if EDITOR
	if (NewFullScreen)
	{
		SDL_MaximizeWindow(SDLWindow);
	}
	else
	{
		SDL_RestoreWindow(SDLWindow);
	}
#else
	if (NewFullScreen) SDL_SetWindowFullscreen(SDLWindow, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
	else SDL_SetWindowFullscreen(SDLWindow, SDL_WINDOW_OPENGL);
	int w, h;
	SDL_GetWindowSize(SDLWindow, &w, &h);
	Graphics::SetWindowResolution(Vector2((float)w, (float)h));
#endif
}
bool Window::GetFullScreen()
{
#if EDITOR
	auto Flag = SDL_GetWindowFlags(SDLWindow);
	auto IsFullScreen = Flag & SDL_WINDOW_MAXIMIZED;
#else
	auto Flag = SDL_GetWindowFlags(SDLWindow);
	auto IsFullScreen = Flag & SDL_WINDOW_FULLSCREEN;
#endif
	return IsFullScreen;
}

Vector2 Window::GetWindowSize()
{
#if SERVER
	return 0;
#endif
	int w, h;
	SDL_GetWindowSize(SDLWindow, &w, &h);
	return Vector2((float)w, (float)h);
}

#endif