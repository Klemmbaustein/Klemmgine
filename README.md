# Klemmgine

A simple 3D game engine written in C++ using OpenGL and SDL2 for Windows. It has basic support for scripting in C#.


## Use with pre-built binaries

### Requirements

The .NET 7 SDK is required.

For shaders, I recommend [this Visual Studio extension](https://marketplace.visualstudio.com/items?itemName=DanielScherzer.GLSL2022).

### How to use

1. Download the latest pre-built binaries [here](https://github.com/Klemmbaustein/Klemmgine/releases/latest).


2. Extract the .zip file and run `ProjectGenerator.exe` to create a new project.	

	```cmd

	.\ProjectGenerator.exe -projectName {Your project name}

	```

	The new project files will be put in `EngineDir\Games\{Your project name}\`.

3. Run `EngineDir\Games\{Your project name}\Editor.bat`.

## Build from source:

### Requirements

Visual Studio 2022 is required. For shaders, I recommend [this extension](https://marketplace.visualstudio.com/items?itemName=DanielScherzer.GLSL2022).

The required workloads are:

- Desktop developement with C++

- Desktop developement with C#

### How to build

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

	The new project files will be put in `EngineDir\Games\{Your project name}\`.



4. Open the generated solution file.

	```cmd

	start devenv "Games\{Your project name}\{Your project name}.sln"

	```



5. Set the project configuration of your project to `Editor` and press F5 to run the editor.