#include "Log.h"
#include "SLNGenerator.h"
#include <fstream>
#include <iostream>
#include "Util.h"
#include "CMakeGenerator.h"
#include <Application.h>
#include <UI/UIBackground.h>
#include <UI/UIScrollBox.h>
#include <UI/UIText.h>
#include <UI/UIButton.h>
#include <UI/UITextField.h>
#include "EngineGeneration.h"

namespace Background
{
	std::thread* BackgroundThread = nullptr;
	std::atomic<float> BackgroundProgress = 0;
	std::string BackgroundTask;
}

int CreateProject(std::string ProjectName, bool WithTemplate)
{
	Background::BackgroundProgress = 0.0;
	Background::BackgroundTask = "Creating Project...";
	if (ProjectName.empty())
	{
		Background::BackgroundTask = "Creating Project... (Project name emtpy)";
		Util::Notify("Please enter a project name");
		Background::BackgroundProgress = 1.0;
		return 1;
	}

	std::string ProjectTemplate;
	if (WithTemplate)
	{
		ProjectTemplate = Util::ShowSelectFolderDialog();
		if (ProjectTemplate.empty())
		{
			Background::BackgroundTask = "Creating Project... (Template selection cancelled)";
			Background::BackgroundProgress = 1.0;
			return 1;
		}
	}

	Log::Print("Beginning project setup.");
	if (!std::filesystem::exists("../Components/lib/Engine_Editor.lib"))
	{
		Background::BackgroundTask = "Creating Project... (Missing libraries)";
		Log::Print("Warning: Missing libraries detected in Components/lib/. Please build the libraries before starting project setup.", Log::E_WARNING);
		if (Util::Ask("Warning: Missing libraries detected in Components/lib/. Please build the libraries before starting project setup. Continue?", {"y", "n"}) == "n")
		{
			Background::BackgroundProgress = 1.0;
			return 1;
		}
	}
	if (!std::filesystem::exists("../paths.txt"))
	{
		Log::Print("'paths.txt' does not exist", Log::E_ERROR);
		Background::BackgroundTask = "Creating Project... (Project name emtpy)";
		Util::Notify("'paths.txt' does not exist. This should never happen.");
		Background::BackgroundProgress = 1.0;
		return 1;
	}
	std::ifstream PathsFile = std::ifstream("../paths.txt");
	std::map<std::string, std::string> Paths;
	while (!PathsFile.eof())
	{
		char ReadBuffer[MAX_PATH_LENGTH];
		PathsFile.getline(ReadBuffer, MAX_PATH_LENGTH * sizeof(char));
		std::string ReadString = ReadBuffer;
		size_t Seperator = ReadString.find_first_of("=");
		std::string a = ReadString.substr(0, Seperator), b = ReadString.substr(Seperator + 1);
		Paths.insert(std::pair(a, b));
	}
	try
	{
		if (std::filesystem::exists("../Games/" + ProjectName))
		{
			Background::BackgroundTask = "Creating Project... (Project already exists)";
			Log::Print(ProjectName + " already exists!", Log::E_WARNING);
			if (Util::Ask(ProjectName + " already exists. Replace old project?", { "y", "n" }) == "n")
			{
				Background::BackgroundProgress = 1.0;
				return 0;
			}
			std::filesystem::remove_all("../Games/" + ProjectName);
			Log::Print("Removed folder Games/" + ProjectName);
		}
	}
	catch (std::exception& e)
	{
		Log::Print("Error while copying files: " + std::string(e.what()), Log::E_ERROR);
		throw "Copy failed";
	}
	Background::BackgroundProgress = 0.2;
	std::string compiler = "1";
	try
	{
		Background::BackgroundTask = "Creating Project... (Copying)";
		std::filesystem::create_directories("../Games/" + ProjectName + "/Content");
		Log::Print("Created folder Games/" + ProjectName);

		Util::CopyFolderContent("../Tools/ProjectGenerator/DefaultProjectFiles", "../Games/" + ProjectName, {}, &Background::BackgroundProgress, 0.2);

#if __linux__
		Util::CopyFolderContent("../Tools/ProjectGenerator/LinuxFiles", "../Games/" + ProjectName, {}, &Background::BackgroundProgress, 0.1);
#elif _WIN32
		Util::CopyFolderContent("../Tools/ProjectGenerator/WindowsFiles", "../Games/" + ProjectName, {}, &Background::BackgroundProgress, 0.2);
		Log::Print("Successfully copied default files");
	}
	catch (std::exception& e)
	{
		Util::Notify("Error while copying files: " + std::string(e.what()));
		Background::BackgroundTask = "Creating Project... (COPY ERROR)";
		Background::BackgroundProgress = 1;
		return 0;
	}

	if (WithTemplate)
	{
		Util::CopyFolderContent(ProjectTemplate, "../Games/" + ProjectName, {}, &Background::BackgroundProgress, 0.2);
	}

	if (compiler == "1")
	{
		try
		{
			Background::BackgroundTask = "Creating Project... (Generating VS project files)";
			Background::BackgroundProgress = 0.8;
			sln::GenerateSolution("../Games/" + ProjectName + "/", ProjectName, Paths,
				Util::GetAllFilesInFolder("../Games/" + ProjectName + "/Code", true));
			Background::BackgroundTask = "Creating Project... (Done)";
			Background::BackgroundProgress = 1;
			return 0;
		}
		catch (Exception& e)
		{
			Background::BackgroundTask = "Creating Project... (SLN error)";
			Util::Notify("Error while trying to create a solution: " + e.What());
			Background::BackgroundProgress = 1;
			return 1;
		}
		catch (std::exception& e)
		{
			Background::BackgroundTask = "Creating Project... (SLN error)";
			Util::Notify("Error while trying to create a solution: " + std::string(e.what()));
			Background::BackgroundProgress = 1;
			return 1;
		}
	}
	if (compiler == "2")
	{
		CMake::WriteCMakesList("../Games/" + ProjectName + "/", ProjectName, Paths,
			Util::GetAllFilesInFolder("../Games/" + ProjectName + "/Code", true));
	}
	Background::BackgroundTask = "Creating Project... (Done)";
	Background::BackgroundProgress = 1;
	return 1;
#endif
}




struct Project
{
	std::string Name;
	std::string Path;
};

TextRenderer* Text = nullptr;

UIBackground* ProjectInfoBackground = nullptr;

// open project: windows sln: system(("cd " + Projects[i].Path + " && start " + Projects[i].Name + ".sln").c_str());
Project SelectedProject;

void ExportProject(Project p)
{
	try
	{
		Background::BackgroundProgress = 0;
		Background::BackgroundTask = "Exporting... (File dialog)";
		std::string To = Util::ShowSelectFolderDialog();
		if (!std::filesystem::exists(To))
		{
			Background::BackgroundTask = "Exporting... (File dialog cancelled)";
			Background::BackgroundProgress = 1;
			return;
		}

		for (const auto& entry : std::filesystem::directory_iterator(To))
		{
			std::filesystem::remove_all(entry.path());
		}
		Background::BackgroundProgress = 0.1;
		Background::BackgroundTask = "Exporting... (Copying files)";
		Util::CopyFolderContent(SelectedProject.Path, To, {".vs"}, &Background::BackgroundProgress, 0.5);
		Background::BackgroundProgress = 0.6;
		Background::BackgroundTask = "Exporting... (Removing files)";
		std::set<std::string> ItemsToDelete =
		{
			"Build",
			"EditorContent",
			"x64",
			"*.dll",
			"*.exe",
			".gitignore",
			"*.sln",
			"*.vcxproj",
			"*.filters",
			"*.user",
			"FindObjects.bat"
		};
		float ExportAddition = 1.f / ItemsToDelete.size();
		for (auto& i : ItemsToDelete)
		{
			Background::BackgroundProgress += ExportAddition / 5.f;
			if (i[0] != '*' && std::filesystem::exists(To + "/" + i))
			{
				std::filesystem::remove_all(To + "/" + i);
			}
		}

		for (const auto& entry : std::filesystem::directory_iterator(To))
		{
			if (ItemsToDelete.contains("*." + Util::GetExtension(entry.path().string())))
			{
				std::filesystem::remove_all(entry.path());
			}
		}
		Background::BackgroundTask = "Exporting... (Done)";
		Background::BackgroundProgress = 1;
	}
	catch (std::exception& e) { Log::Print(e.what()); }
}

void GenerateProjectInfo(Project p)
{
	ProjectInfoBackground->DeleteChildren();
	ProjectInfoBackground->AddChild((new UIText(0.6, 0, "Project: " + p.Name, Text))->SetPadding(0.03, 0.01, 0.02, 0.01));
	ProjectInfoBackground->AddChild((new UIText(0.3, 0, p.Path, Text))->SetPadding(0.03, 0.01, 0.02, 0.01));
	SelectedProject = p;
	if (std::filesystem::exists(p.Path + "/Code/GENERATED"))
	{
		size_t NumClasses = 0;

		for (const auto& entry : std::filesystem::directory_iterator(p.Path + "/Code/GENERATED"))
		{
			NumClasses++;
		}
		ProjectInfoBackground->AddChild((new UIText(0.3, 0, std::to_string(NumClasses - 4) + " C++ objects", Text))->SetPadding(0.03, 0.01, 0.02, 0.01));
	}

	ProjectInfoBackground->AddChild((new UIBox(true, 0))

		->AddChild((new UIButton(true, 0, Vector3f32(0, 0.8, 0),
			[]() {system(("cd " + SelectedProject.Path + " && start " + SelectedProject.Name + ".sln").c_str()); Application::Quit = true; }))
			->AddChild((new UIText(0.4, 0, "  Open  ", Text))
				->SetPadding(0.04))
			->SetBorder(UIBox::E_ROUNDED, 0.75)
			->SetPadding(0.1, 0.04, 0.04, 0.04))

		->AddChild((new UIButton(true, 0, Vector3f32(1, 0.8, 0), []()
			{
				if (!Background::BackgroundThread)
				{
					Background::BackgroundThread = new std::thread(ExportProject, SelectedProject);
				}
			}))
			->AddChild((new UIText(0.4, 0, "Export", Text))
				->SetPadding(0.04))
			->SetBorder(UIBox::E_ROUNDED, 0.75)
			->SetPadding(0.1, 0.04, 0.04, 0.04)));
}

std::vector<UIButton*> ProjectButtons;
std::vector<Project> Projects;
UITextField* NewProjectNameField = nullptr;
UIScrollBox* ProjectListBackground = nullptr;

void UpdateProjects()
{
	ProjectButtons.clear();
	Projects.clear();
	if (!std::filesystem::exists("../Games"))
	{
		std::filesystem::create_directory("../Games");
	}
	for (const auto& entry : std::filesystem::directory_iterator("../Games"))
	{
		if (entry.is_directory() && std::filesystem::exists(entry.path().string() + "/BuildTool.exe"))
		{
			std::string ProjectName = entry.path().string();
			ProjectName = ProjectName.substr(ProjectName.find_last_of("\\/") + 1);
			Projects.push_back(Project(ProjectName, std::filesystem::canonical(entry.path()).string()));
		}
	}
	ProjectListBackground->DeleteChildren();
	for (auto& i : Projects)
	{
		UIButton* b = (new UIButton(true, 0, 0.75, []()
			{
				for (size_t i = 0; i < ProjectButtons.size(); i++)
				{
					if (ProjectButtons[i]->IsBeingHovered())
					{
						GenerateProjectInfo(Projects[i]);
					}
				}
			}));
		ProjectListBackground->AddChild(b->SetMinSize(Vector2f(0.5, 0.1))
			->SetBorder(UIBox::E_ROUNDED, 0.75)
			->AddChild((new UIText(0.3, 0, i.Name, Text))
				->SetPadding(0.04, 0.04, 0.1, 0)));
		ProjectButtons.push_back(b);
	}
}

void InitProjectUI()
{
	ProjectListBackground = new UIScrollBox(false, Vector2f(-0.9, -0.75), 15);
	ProjectListBackground->Align = UIBox::E_REVERSE;
	ProjectListBackground->SetMinSize(Vector2f(0.75, 1.5));
	ProjectListBackground->SetMaxSize(Vector2f(0.75, 1.5));
	UIText* t = new UIText(0.6, 0, "Projects", Text);
	t->SetPosition(Vector2f(-0.9, 0.8));

	ProjectInfoBackground = new UIBackground(false, Vector2f(-0.2, -0.9), 0.85, Vector2f(1.1, 1.8));
	ProjectInfoBackground->SetBorder(UIBox::E_ROUNDED, 2);
	ProjectInfoBackground->Align = UIBox::E_REVERSE;

	NewProjectNameField = (new UITextField(true, Vector2f(-0.95, -0.84), 0.2, Text, []() {}));
	NewProjectNameField
		->SetTextSize(0.25)
		->SetHintText("Project name")
		->SetMinSize(Vector2f(0.45, 0.05))
		->SetBorder(UIBox::E_ROUNDED, 0.75);

	(new UIBox(true, Vector2f(-0.95, -0.75)))
		->AddChild((new UIButton(true, 0, Vector3f32(0, 0.85, 0), []() {
		if (!Background::BackgroundThread)
		{
			Background::BackgroundThread = new std::thread(CreateProject, NewProjectNameField->GetText(), false);
		}
			}))
			->SetPadding(0.01, 0.01, 0, 0.01)
				->AddChild((new UIText(0.3, 0, "New project", Text))
					->SetPadding(0.03, 0.03, 0.02, 0.02))
				->SetMinSize(Vector2f(0.2, 0.08))
				->SetBorder(UIBox::E_ROUNDED, 0.5))
		->AddChild((new UIButton(true, 0, Vector3f32(1, 0.8, 0), []() {
				if (!Background::BackgroundThread)
				{
					Background::BackgroundThread = new std::thread(CreateProject, NewProjectNameField->GetText(), true);
				}
			}))
			->SetPadding(0.01, 0.01, 0.01, 0.01)
				->AddChild((new UIText(0.3, 0, "Import project", Text))
					->SetPadding(0.03, 0.03, 0.02, 0.02))
				->SetMinSize(Vector2f(0.2, 0.08))
				->SetBorder(UIBox::E_ROUNDED, 0.5));

	UpdateProjects();
}


int main(int argc, char** argv)
{
	std::filesystem::current_path("Launcher");
	Application::Initialize("Project manager", Application::NO_RESIZE_BIT, Vector2ui(1200, 900));
	new UIBackground(true, -1, 0.9, Vector2f(2));

	Text = new TextRenderer();
	bool IsFirstInstall = !std::filesystem::exists("../cppinfo.txt");
	if (IsFirstInstall)
	{
		Installation::ManageFirstInstall(Text);
	}
	else
	{
		InitProjectUI();
	}
	new UIBackground(true, -1, Vector3f32(0.6), Vector2f(2, 0.05));
	UIBackground* ProgressBar = new UIBackground(true, Vector2f(-1.1, -1), Vector3f32(0.8, 0.3, 0), Vector2f(0, 0.1));
	ProgressBar->SetBorder(UIBox::E_ROUNDED, 0.75);
	UIText* BackgroundTaskText = new UIText(0.3, 0, "", Text);
	BackgroundTaskText->SetPosition(Vector2f(-0.99, -0.98));
	float TaskFinishedTimer = -1;

	while (!Application::Quit)
	{
		ProgressBar->SetOpacity(TaskFinishedTimer == -1 ? 1 : 1 - TaskFinishedTimer);
		BackgroundTaskText->SetOpacity(TaskFinishedTimer == -1 ? 1 : 1 - TaskFinishedTimer);
		ProgressBar->SetMinSize(Vector2f(Background::BackgroundProgress * 2.1 + 0.1, 0.05));
		BackgroundTaskText->SetText(Background::BackgroundTask);
		if (Background::BackgroundProgress == 1 && TaskFinishedTimer == -1)
		{
			Background::BackgroundThread->join();
			delete Background::BackgroundThread;
			Background::BackgroundThread = nullptr;
			TaskFinishedTimer = 0;
			if (IsFirstInstall)
			{
				Installation::UpdateLibrayPaths();
			}
			else
			{
				UpdateProjects();
			}
		}
		if (TaskFinishedTimer < 1 && TaskFinishedTimer >= 0)
		{
			TaskFinishedTimer += Application::DeltaTime;
		}
		else if (TaskFinishedTimer >= 0)
		{
			TaskFinishedTimer = -1;
			Background::BackgroundProgress = 0;
			Background::BackgroundTask = "";
		}

		//Log::Print(std::to_string(ExportProgress));
		Application::SetActiveMouseCursor(UI::HoveredButton ? Application::CURSOR_HAND : Application::CURSOR_NORMAL);
		Application::UpdateWindow();
	}
}

int WinMain()
{
	main(__argc, __argv);
}