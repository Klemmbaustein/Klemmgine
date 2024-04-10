#if !SERVER
#include "InputSubsystem.h"
#include <Engine/Gamepad.h>
#include <Engine/Input.h>
#include <SDL.h>
#include <Engine/Application.h>
#include <UI/UIBox.h>
#include <Rendering/Graphics.h>
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Utility/StringUtility.h>
#include <Engine/Log.h>
#include <cstring>

namespace Input
{
	extern bool Keys[351];
}

static void MoveTextIndex(int Amount, bool RespectShiftPress = true)
{
	TextInput::TextIndex = std::max(std::min(TextInput::TextIndex + Amount, (int)TextInput::Text.size()), 0);
	if ((!Input::IsKeyDown(Input::Key::LSHIFT) && !Input::IsKeyDown(Input::Key::RSHIFT)) || !RespectShiftPress)
	{
		TextInput::TextSelectionStart = TextInput::TextIndex;
	}
}

static void DeleteSelection()
{
	int Difference = std::abs(TextInput::TextSelectionStart - TextInput::TextIndex);

	TextInput::Text.erase(std::min(TextInput::TextIndex, TextInput::TextSelectionStart), Difference);

	TextInput::TextIndex = std::min(TextInput::TextIndex, TextInput::TextSelectionStart);

	TextInput::TextSelectionStart = TextInput::TextIndex;
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

	PollInput();
	if (Input::CursorVisible || !Application::WindowHasFocus())
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);
		Input::MouseMovement = 0;
		Input::MouseLocation = Application::GetCursorPosition();
	}
	else
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}

	Input::GamepadUpdate();
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
				MoveTextIndex(-1);
				break;
			case SDLK_RIGHT:
				MoveTextIndex(1);
				break;
			case SDLK_x:
				if (!TextInput::PollForText || (!Input::IsKeyDown(Input::Key::LCTRL) && !Input::IsKeyDown(Input::Key::RCTRL)))
					break;
				SDL_SetClipboardText(TextInput::GetSelectedTextString().c_str());
				[[fallthrough]];
			case SDLK_BACKSPACE:
				if (TextInput::PollForText && TextInput::Text.length() > 0)
				{
					if (TextInput::TextIndex == TextInput::Text.size())
					{
						int Difference = std::abs(TextInput::TextSelectionStart - TextInput::TextIndex);

						for (int i = 0; i < Difference; i++)
						{
							TextInput::Text.pop_back();
						}

						if (Difference == 0)
						{
							TextInput::Text.pop_back();
						}
					}
					else if (TextInput::TextIndex > 0 || TextInput::TextSelectionStart > 0)
					{
						if (TextInput::TextSelectionStart == TextInput::TextIndex)
						{
							TextInput::Text.erase(--TextInput::TextIndex, 1);
						}
						else
						{
							DeleteSelection();
						}
					}
					TextInput::SetTextIndex(std::max(std::min(TextInput::TextIndex, (int)TextInput::Text.size()), 0), true);
				}
				break;
			case SDLK_DELETE:
				if (TextInput::PollForText && TextInput::TextIndex < TextInput::Text.size() && TextInput::TextIndex >= 0)
				{
					if (TextInput::TextSelectionStart == TextInput::TextIndex)
					{
						TextInput::Text.erase(TextInput::TextIndex, 1);
					}
					else
					{
						DeleteSelection();
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
			case SDLK_c:
				if (TextInput::PollForText && (Input::IsKeyDown(Input::Key::LCTRL) || Input::IsKeyDown(Input::Key::RCTRL)))
				{
					SDL_SetClipboardText(TextInput::GetSelectedTextString().c_str());
				}
				break;
			case SDLK_v:
				if (TextInput::PollForText && (Input::IsKeyDown(Input::Key::LCTRL) || Input::IsKeyDown(Input::Key::RCTRL)))
				{
					DeleteSelection();
					std::string ClipboardText = SDL_GetClipboardText();
					if (TextInput::TextIndex < TextInput::Text.size())
					{
						TextInput::Text.insert(TextInput::TextIndex, ClipboardText);
					}
					else
					{
						TextInput::Text.append(ClipboardText);
					}
					MoveTextIndex((int)ClipboardText.size(), false);
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
					DeleteSelection();
					TextInput::Text.insert(TextInput::TextIndex, std::string(Event.text.text));
					MoveTextIndex((int)strlen(Event.text.text), false);
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
#endif