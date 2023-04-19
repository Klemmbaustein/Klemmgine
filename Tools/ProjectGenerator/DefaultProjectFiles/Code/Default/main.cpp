#include <Engine/OS.h>
#include <Sound/Sound.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include <Engine/Log.h>
#include <World/Stats.h>
#include <Engine/Application.h>

int Initialize(int argc, char** argv);

void HandleError(std::string ErrorMessage)
{
	OS::SetConsoleWindowVisible(true);
	SDL_DestroyWindow(Application::Window);
	Sound::StopAllSounds();
	Sound::End();
	OS::ClearConsoleWindow();
	std::cout << "Thrown uncaught exception at: " << Debugging::EngineStatus << ": \n";
	std::cout << ErrorMessage;
	std::cout << "\nPress enter to display engine log\n";
	std::cin.get();
	std::cout << "Engine Log:\n";
	for (Log::Message i : Log::Messages)
	{
		std::cout << "    " <<
			std::string(i.Amount != 0 ?
				i.Text + std::string(" (x") + std::to_string(i.Amount + 1) + std::string(")") :
				i.Text) << "\n";
	}
	std::cout << "\nPress enter to continue";
	std::cin.get();
}

int main(int argc, char** argv)
{
	try
	{
		return Initialize(argc, argv);
	}
	catch (const std::exception& e)
	{
		HandleError(e.what());
	}
	catch (const char* e)
	{
		HandleError(e);
	}
}