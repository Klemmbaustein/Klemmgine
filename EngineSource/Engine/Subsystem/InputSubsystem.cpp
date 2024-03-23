#include "InputSubsystem.h"
#include <Engine/Gamepad.h>
#include <Engine/Input.h>
#include <SDL.h>
#include <Engine/Application.h>
#include <UI/UIBox.h>
#include <Rendering/Graphics.h>
#include <UI/EditorUI/EditorUI.h>

namespace Input
{
	extern bool Keys[351];
}

InputSubsystem::InputSubsystem()
{
	Name = "InputSys";
	for (int i = 0; i < 322; i++)
	{
		Input::Keys[i] = false;
	}
}

void InputSubsystem::Update()
{
	Input::IsLMBClicked = false;
	Input::IsRMBClicked = false;
	Input::MouseMovement = Vector2();

	if (Input::CursorVisible || !Application::WindowHasFocus())
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}
	else
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	if (!Application::WindowHasFocus())
	{
		Input::MouseMovement = 0;
	}

	PollInput();
	Input::GamepadUpdate();
	Input::MouseLocation = Application::GetCursorPosition();
}

void InputSubsystem::PollInput()
{
	SDL_Event Event;
	while (SDL_PollEvent(&Event))
	{
		if (Event.type == SDL_QUIT)
		{
#if EDITOR
			Application::EditorInstance->OnLeave(Application::Quit);
#else
			Application::Quit();
#endif
		}
		else if (Event.type == SDL_JOYBUTTONDOWN
			|| Event.type == SDL_JOYBUTTONUP
			|| Event.type == SDL_JOYAXISMOTION
			|| Event.type == SDL_JOYHATMOTION)
		{
			Input::HandleGamepadEvent(&Event);
		}
		else if (Event.type == SDL_MOUSEMOTION)
		{
			Input::MouseMovement += Vector2(Event.motion.xrel / 12.f, -Event.motion.yrel / 12.f);
		}
		else if (Event.type == SDL_JOYDEVICEADDED)
		{
			Input::AddGamepad(Event.jdevice.which);
		}
		else if (Event.type == SDL_KEYDOWN)
		{
			if (Event.key.keysym.sym < 128)
			{
				Input::Keys[Event.key.keysym.sym] = true;
			}
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
				if (TextInput::PollForText && (Input::IsKeyDown(Input::Key::LCTRL) || Input::IsKeyDown(Input::Key::RCTRL)))
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
			{
				Input::Keys[Event.key.keysym.sym] = false;
			}
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
				Input::IsRMBClicked = true;
				TextInput::PollForText = false;
				break;
			case SDL_BUTTON_LEFT:
				Input::IsLMBClicked = true;
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
				if (Event.text.text[0] >= 32)
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
				for (ScrollObject* s : UIBox::ScrollObjects)
				{
					if (Event.wheel.y < 0)
						s->ScrollUp();
					else
						s->ScrollDown();
				}
				if (ScrollDistance < 0)
				{
					ScrollDistance++;
				}
				else
				{
					ScrollDistance--;
				}
			}
		}
	}
}
