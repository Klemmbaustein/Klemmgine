# Klemmgine

A simple 3D game engine written in C++ using OpenGL and SDL2. It has basic support for scripting in C#.

## Requirements

The engine is currently only avaliable on Windows. 
The code itself compiles on Linux, but there is no way to build it yet.

Visual Studio 2022 is required for C++20 support. (Jetbrains Rider probably also works.)

The required workloads are:
- Desktop developement with C++
- Desktop developement with C#

For writing shaders, I recommend 
[this extension](https://marketplace.visualstudio.com/items?itemName=DanielScherzer.GLSL2022).

## Install from pre-built binaries

1. Download the latest pre-built binaries [here](https://github.com/Klemmbaustein/Klemmgine/releases/latest).

2. Extract the .zip file and run `ProjectGenerator.exe` to create a new project.	
	```cmd
	ProjectGenerator.exe -projectName {Your project name}
	```
	The new project files will be put in `EngineDir\Games\{Your project name}\`

3. Open the generated solution file.
	```cmd
	start devenv "Games\{Your project name}\{Your project name}.sln"
	```
4. Set the project configuration of your project to `Editor` and press F5 to run the editor.

## Buld from source:

1. Clone the repository and it's submodules.
	```cmd
	git clone https://github.com/Klemmbaustein/Klemmgine.git --recurse-submodules
	```

2. In the engine directory, run [setup.ps1](./setup.ps1)
	using the Visual Studio Developer Powershell. This will build the engine and it's dependencies.
	```cmd
	.\setup.ps1
	```

3. Run the newly built `ProjectGenerator.exe` to create a new project.	
	```cmd
	ProjectGenerator.exe -projectName {Your project name}
	```
	The new project files will be put in `EngineDir\Games\{Your project name}\`

4. Open the generated solution file.
	```cmd
	start devenv "Games\{Your project name}\{Your project name}.sln"
	```

5. Set the project configuration of your project to `Editor` and press F5 to run the editor.