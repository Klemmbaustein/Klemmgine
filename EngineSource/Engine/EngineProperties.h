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
#define VERSION_STRING "1.8.1"
#define OPENGL_MIN_REQUIRED_VERSION "GL_VERSION_4_2"

/**
* @defgroup Getting-Started
* 
* # Sections:
* 
* - <a href="#install">Editor guide</a>
* - <a href="#editor">Editor guide</a>
*
* <h1 id="install">Installing the editor</h1>
* 
* ## With pre-built binaries
* 
* With this method, you can only write game logic in C#.
* This requires the .NET 8 SDK to be installed.
* 
* Download the editor [here](https://github.com/Klemmbaustein/Klemmgine/releases/latest).
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
*	```sh
*	sudo apt-get install libsdl2-dev
*   sudo apt-get install libglew-dev
*   sudo apt-get install libopenal-dev
*   ```
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
* .\ProjectGenerator.exe -projectName {Your project name}
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
* #### Linux
* 
* @todo Make the Linux build process not horrible.
*
* 
* 
*/


/**
* @mainpage Klemmgine documentation
*
* ## Links
* - Getting started: @ref Getting-Started
* - C# documentation: @ref CSharp
*/
