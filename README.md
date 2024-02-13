# Klemmgine

[![Build binaries](https://github.com/Klemmbaustein/Klemmgine/actions/workflows/build.yml/badge.svg)](https://github.com/Klemmbaustein/Klemmgine/actions/workflows/build.yml)

A simple 3D game engine written in C++ using OpenGL and SDL2 for Windows and Linux. It has basic support for scripting in C#.

## Use with pre-built binaries

### Requirements

The .NET 8 SDK is required.

For shaders, I recommend [this Visual Studio extension](https://marketplace.visualstudio.com/items?itemName=DanielScherzer.GLSL2022).

### How to use

1. Download the latest pre-built binaries [here](https://github.com/Klemmbaustein/Klemmgine/releases/latest).


2. Extract the .zip file and run `ProjectGenerator.exe` to create a new project.	

    ```sh
    # Windows
    ProjectGenerator.exe -projectName {Your project name}
    # Linux
    ./ProjectGenerator -projectName {Your project name}
    ```

    The new project files will be put in `EngineDir/Games/{Your project name}/`.

3. Run `EngineDir\Games\{Your project name}\Editor.bat` on windows or `EngineDir/Games/{Your project name}/Editor.sh` on linux.

## Build from source:

### Windows

- #### Requirements

  Visual Studio 2022 is required. For shaders, I recommend [this extension](https://marketplace.visualstudio.com/items?itemName=DanielScherzer.GLSL2022).

  The required workloads are:

  - Desktop developement with C++

  - Desktop developement with C#

- #### How to build

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
     ProjectGenerator.exe -projectName {Project name}
     ```

     The new project files will be put in `EngineDir\Games\{Project name}\`.

  4. Open the generated solution file.

     ```cmd
     start devenv "Games\{Project name}\{Project name}.sln"
     ```

  5. Set the project configuration of your project to `Editor` and press F5 to run the editor.

### Linux

- #### Requirements:
  
  - The SDL2, OpenAL and GLEW developement libraries.
    
    With apt: 
    ```sh
    sudo apt-get install libsdl2-dev
    sudo apt-get install libglew-dev
    sudo apt-get install libopenal-dev
    ```
    
  - The [KlemmBuild](https://github.com/Klemmbaustein/KlemmBuild) build system is required to build the engine.

- #### How to build
  
  1. Clone the repository and it's submodules.

     ```sh
     git clone https://github.com/Klemmbaustein/Klemmgine.git --recurse-submodules
     ``` 
  2. Run the setup script.
     ```sh
     ./setup.sh
     ```
  3. Build the project generator
     ```sh
     KlemmBuild engine.kbld -DGenerator
     ```
  4. Run the project generator
     ```sh
     ./ProjectGenerator -projectName {Project name}
     ```
  5. Build and run the project
     ```sh
     cd Games/{Project name}
     KlemmBuild -DEditor
     ./Editor.sh
     ```

## Building documentation

The documentation is generated using Doxygen.
To generate the documentation, run `doxygen` in the engine's root directory, where the doxyfile is located.

> Note: Some Linux distros (like Ubuntu) have an old version of doxygen (1.9 or 1.8)
> in their package repositories that will improperly build the C# documentation.
> To install the latest version of doxygen, download it from [their website](https://www.doxygen.nl/download.html).