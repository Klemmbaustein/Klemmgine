#pragma once
#include <string>
#ifdef EDITOR
#define IS_IN_EDITOR true
#else
#define IS_IN_EDITOR false
#endif
#ifdef RELEASE
#define ENGINE_DEBUG false
#else
#define ENGINE_DEBUG true
#endif
namespace Project
{
	std::string GetStartupScene();
	void OnLaunch();
	extern const char* ProjectName;
}
#define VERSION_STRING "1.9.3"
#define OPENGL_MIN_REQUIRED_VERSION "GL_VERSION_4_2"

/**
* @defgroup Getting-Started Guide
* 
* # Sections:
* 
* - <a href="#install">Installation</a>
* - <a href="#editor">Editor guide</a>
*
* <h1 id="install">Installing the editor</h1>
* 
* ## With pre-built binaries
* 
* With this method, you can only write game logic in C#.
* This requires the .NET 8 SDK to be installed.
* 
* Download the engine [here](https://github.com/Klemmbaustein/Klemmgine/releases/latest).
* 
* ## From source
* 
* With this method, you can write game logic in C++ or optionally C#.
* This can result in smaller games since the C# runtime doesn't need to be shipped.
* You can also modify the engine this way.
* 
* ### Requirements:
* 
* #### Windows:
* 
* - Visual Studio with the following Workloads is required:
*   - Desktop developement with C++
*   - Desktop developement with C#
* 
* #### Linux
* 
* - Install [KlemmBuild](https://github.com/Klemmbaustein/KlemmBuild).
* - Install SDL2, GLEW and OpenAL developement libraries.
*   With `apt`, this would be:
* 
*   ```
*	sudo apt-get install libsdl2-dev libglew-dev libopenal-dev
*   ```
* 
* - Install the .NET 8 SDK.
* 
* ### How to build and run:
* 
* #### Windows
* 
* Clone the repository and it's submodules.
* 
* ```pwsh
* git clone https://github.com/Klemmbaustein/Klemmgine.git --recurse-submodules
* ```
* 
* In the engine directory, run setup.ps1
* using the Visual Studio Developer Powershell. This will build the engine and it's dependencies.
* 
* ```pwsh
* .\setup.ps1
* ```
* 
* Run the newly built 'ProjectGenerator.exe' to create a new project.
* 
* ```pwsh
* ProjectGenerator.exe -projectName {Your project name}
* ```
* 
* The new project files will be put in `EngineDir\Games\{Your project name}\`.
* 
* Open the generated solution file.
* 
* ```pwsh
* start devenv "Games\{Your project name}\{Your project name}.sln"
* ```
* 
* Set the project configuration of your project to 'Editor' and press F5 to run the editor.
* 
* 
* #### Linux
* 
* Clone the repository and it's submodules.
* ```sh
* git clone https://github.com/Klemmbaustein/Klemmgine.git --recurse-submodules
* ```
* 
* Run the setup script.
* ```sh
* ./setup.sh
* ```
* 
* Build the project generator
* ```sh
* KlemmBuild engine.kbld -DGenerator
* ```
* 
* Run the project generator
* ```sh
* ./ProjectGenerator -projectName {Project name}
* ```
* 
* Build and run the project
* ```sh
* cd Games/{Project name}
* KlemmBuild -DEditor
* ./Editor.sh
* ```
* 
* <h1 id="editor">Editor guide</h1>
* 
* @todo Finish editor documentation.
*/


/**
* @mainpage Klemmgine documentation
*
* A simple 3D game engine written in C++ using OpenGL and SDL2 for Windows. It has basic support for scripting in C#.
* 
* ## Links
* - Getting started: @ref Getting-Started
* - C# documentation: @ref CSharp
*/
