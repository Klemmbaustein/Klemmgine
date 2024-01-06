#if EDITOR
#include "AboutWindow.h"
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Log.h>
#include <Engine/EngineProperties.h>
#include <CSharp/CSharpInterop.h>
#include <UI/UIScrollBox.h>
#include <unordered_map>

// Template hell
static std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>> Credits =
{ 
	{"By:", {}},
	{"Klemmbaustein", {std::pair("Github", "https://github.com/Klemmbaustein")}},
	{"3rd party libraries:", {}},
	{"stb_image, stb_truetype", {std::pair("Github", "https://github.com/nothings/stb")}},
	{"assimp", {std::pair("Website", "https://assimp.org"), std::pair("Github", "https://github.com/assimp/assimp")}},
	{"glew, glew_cmake", {std::pair("glew", "https://github.com/nigels-com/glew"), std::pair("glew_cmake", "https://github.com/Perlmint/glew-cmake")}},
	{"glm", {std::pair("Github", "https://github.com/g-truc/glm")}},
	{"OpenAL soft", {std::pair("Github", "https://github.com/kcat/openal-soft")}},
	{"SDL2, SDL2_net", {
		std::pair("Website", "https://www.libsdl.org/"),
		std::pair("SDL2", "https://github.com/libsdl-org/SDL"),
		std::pair("SDL2_net", "https://github.com/libsdl-org/SDL_net")
	}}
};

AboutWindow::AboutWindow()
	: EditorPopup(0, Vector2(0.5f, 0.6f), "About")
{	
	SetOptions({PopupOption("OK")});

	ContentBox = new UIBox(UIBox::Orientation::Vertical, 0);
	ContentBox
		->SetPadding(0)
		->SetMinSize(0.3f);
	PopupBackground->AddChild(ContentBox);

#if _WIN32
	std::string OsString = "Windows (x64)";
#else
	std::string OsString = "Linux (x64)";
#endif

	ContentBox->AddChild((new UIBox(UIBox::Orientation::Horizontal, 0))
		->SetPadding(0)
		->AddChild((new UIBackground(UIBox::Orientation::Horizontal, 0, 1, 0.1f))
			->SetUseTexture(true, EditorUI::Textures[15])
			->SetPadding(0.01f)
			->SetSizeMode(UIBox::SizeMode::PixelRelative))
		->AddChild((new UIBox(UIBox::Orientation::Vertical, 0))
			->SetPadding(0)
			->AddChild((new UIText(0.5f, EditorUI::UIColors[2], "Klemmgine Editor v" + std::string(VERSION_STRING), EditorUI::Text))
				->SetPadding(0.005f))
			->AddChild((new UIText(0.4f, EditorUI::UIColors[2], "  For " + OsString, EditorUI::Text))
				->SetPadding(0.005f))
#if ENGINE_CSHARP
			->AddChild((new UIText(0.4f, EditorUI::UIColors[2], std::string("  C#: ") + (CSharp::GetUseCSharp() ? "Yes" : "Disabled"), EditorUI::Text))
				->SetPadding(0.005f))
#else
			->AddChild((new UIText(0.4f, EditorUI::UIColors[2], "  C#: No", EditorUI::Text))
				->SetPadding(0.005f))
#endif
#if ENGINE_NO_SOURCE
			->AddChild((new UIText(0.4f, EditorUI::UIColors[2], "  With pre-built binaries", EditorUI::Text))
				->SetPadding(0.005f))
			->AddChild((new UIText(0.4f, EditorUI::UIColors[2], "  Build date: "
				+ std::string(__DATE__)
				+ " - "
				+ std::string(__TIME__), EditorUI::Text))
				->SetPadding(0.005f))
#endif
	));

	UIScrollBox* CreditsBox = new UIScrollBox(UIBox::Orientation::Vertical, 0, true);
	ContentBox->AddChild(CreditsBox
		->SetPadding(0));

	int ButtonIndex = 1;

	for (auto& i : Credits)
	{
		CreditsBox->AddChild((new UIText(0.45f, EditorUI::UIColors[2], i.first, EditorUI::Text))
			->SetPadding(i.second.empty() ? 0.02f : 0.005f, 0.005f, i.second.empty() ? 0.005f : 0.02f, 0.005f));

		UIBox* ButtonsBox = new UIBox(UIBox::Orientation::Horizontal, 0);
		CreditsBox->AddChild(ButtonsBox
			->SetPadding(0));

		for (auto& entry : i.second)
		{
			ButtonsBox->AddChild((new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2], this, ButtonIndex++))
				->SetBorder(UIBox::BorderType::Rounded, 0.2f)
				->AddChild((new UIText(0.4f, 1 - EditorUI::UIColors[2], entry.first, EditorUI::Text))
					->SetPadding(0.005f))
				->SetPadding(0.005f, 0.005f, i.second.empty() ? 0.005f : 0.02f, 0.005f));
		}

		if (i.second.empty())
		{
			ButtonsBox->AddChild((new UIBackground(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2], Vector2(0.5f, 0.003f)))
				->SetPadding(0));
		}
	}
	CreditsBox->SetMinSize(Vector2(0.5f, 0.365f));
	CreditsBox->SetMaxSize(Vector2(0.5f, 0.365f));
}

AboutWindow::~AboutWindow()
{
}

void AboutWindow::OnButtonClicked(int Index)
{
	if (Index == 0)
	{
		delete this;
		return;
	}

	int it = 1;

	for (auto& i : Credits)
	{
		for (auto& entry : i.second)
		{
			if (it++ == Index)
			{
#if _WIN32
				system(("start " + entry.second).c_str());
#else 
				system(("xdg-open " + entry.second).c_str());
#endif
			}
		}
	}
}

void AboutWindow::Tick()
{
	TickPopup();
}
#endif