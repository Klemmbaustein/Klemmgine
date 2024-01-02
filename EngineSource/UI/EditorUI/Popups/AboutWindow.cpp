#if EDITOR && 0
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
	: EditorPanel(Application::EditorInstance->UIColors, 0, Vector2(0.5f, 0.6f), Vector2(0.5f, 0.6f), Vector2(0.5f, 0.6f), true, "About")
{	
	auto* Text = Application::EditorInstance->EngineUIText;

	ButtonBackground = new UIBackground(true, 0, UIColors[0] * 1.5);
	ButtonBackground->SetPadding(0);
	ButtonBackground->SetVerticalAlign(UIBox::Align::Centered);
	ButtonBackground->SetBorder(UIBox::BorderType::DarkenedEdge, 0.2f);

	ButtonBackground->AddChild(
		(new UIButton(true, 0, UIColors[2], this, (int)-1))
		->SetPadding(0.01f)
		->SetBorder(UIBox::BorderType::Rounded, 0.2f)
		->AddChild((new UIText(0.45f, 1 - UIColors[2], "Ok", Text))
			->SetPadding(0.005f)));

	TabBackground->SetVerticalAlign(UIBox::Align::Default);
	TabBackground->AddChild(ButtonBackground);
	ContentBox = new UIBox(false, 0);
	ContentBox
		->SetPadding(0)
		->SetMinSize(0.3f);
	TabBackground->AddChild(ContentBox);

#if _WIN32
	std::string OsString = "Windows (x64)";
#else
	std::string OsString = "Linux (x64)";
#endif

	ContentBox->AddChild((new UIBox(true, 0))
		->SetPadding(0)
		->AddChild((new UIBackground(true, 0, 1, 0.1f))
			->SetUseTexture(true, Application::EditorInstance->Textures[15])
			->SetPadding(0.01f)
			->SetSizeMode(UIBox::SizeMode::PixelRelative))
		->AddChild((new UIBox(false, 0))
			->SetPadding(0)
			->AddChild((new UIText(0.5f, UIColors[2], "Klemmgine Editor v" + std::string(VERSION_STRING), Text))
				->SetPadding(0.005f))
			->AddChild((new UIText(0.4f, UIColors[2], "  " + OsString, Text))
				->SetPadding(0.005f))
#if ENGINE_CSHARP
			->AddChild((new UIText(0.4f, UIColors[2], std::string("  C#: ") + (CSharp::GetUseCSharp() ? "Yes" : "Disabled"), Text))
				->SetPadding(0.005f))
#else
			->AddChild((new UIText(0.4f, UIColors[2], "  C#: No", Text))
				->SetPadding(0.005f))
#endif
#if ENGINE_NO_SOURCE
			->AddChild((new UIText(0.4f, UIColors[2], "  With pre-built binaries", Text))
				->SetPadding(0.005f))
			->AddChild((new UIText(0.4f, UIColors[2], "  Build date: " 
				+ std::string(__DATE__)
				+ " - "
				+ std::string(__TIME__), Text))
				->SetPadding(0.005f))
#endif
	));

	UIScrollBox* CreditsBox = new UIScrollBox(false, 0, true);
	ContentBox->AddChild(CreditsBox
		->SetPadding(0));

	int ButtonIndex = 0;

	for (auto& i : Credits)
	{
		CreditsBox->AddChild((new UIText(0.45f, UIColors[2], i.first, Text))
			->SetPadding(i.second.empty() ? 0.02f : 0.005f, 0.005f, i.second.empty() ? 0.005f : 0.02f, 0.005f));

		UIBox* ButtonsBox = new UIBox(true, 0);
		CreditsBox->AddChild(ButtonsBox
			->SetPadding(0));

		for (auto& entry : i.second)
		{
			ButtonsBox->AddChild((new UIButton(true, 0, UIColors[2], this, ButtonIndex++))
				->SetBorder(UIBox::BorderType::Rounded, 0.2f)
				->AddChild((new UIText(0.4f, UIColors[1], entry.first, Text))
					->SetPadding(0.005f))
				->SetPadding(0.005f, 0.005f, i.second.empty() ? 0.005f : 0.02f, 0.005f));
		}

		if (i.second.empty())
		{
			ButtonsBox->AddChild((new UIBackground(true, 0, UIColors[2], Vector2(0.5f, 0.003f)))
				->SetPadding(0));
		}
	}
	CreditsBox->SetMinSize(Vector2(0.5f, 0.365f));
	CreditsBox->SetMaxSize(Vector2(0.5f, 0.365f));
	UpdateLayout();
}

void AboutWindow::UpdateLayout()
{
	ButtonBackground->SetMinSize(Vector2(TabBackground->GetMinSize().X, 0.075f));
}

AboutWindow::~AboutWindow()
{
}

void AboutWindow::OnButtonClicked(int Index)
{
	if (Index == -1)
	{
		delete this;
		return;
	}

	int it = 0;

	for (auto& i : Credits)
	{
		for (auto& entry : i.second)
		{
			if (it++ == Index)
			{
#if _WIN32
				system(("start " + entry.second).c_str());
#endif
			}
		}
	}
}

void AboutWindow::Tick()
{
	UpdatePanel();
}
#endif