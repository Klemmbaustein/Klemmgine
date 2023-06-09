# Installing the engine

## Visual Studio

This requires Visual Studio 2022. It could probably work with JetBrains Rider.

The engine requires both include files (headers) and libraries of:

* [glm (header only)](https://github.com/g-truc/glm/)
* [SDL2](https://github.com/libsdl-org/SDL/releases/tag/release-2.24.2)
* [glew](https://github.com/nigels-com/glew)
* [OpenAL Soft](https://github.com/kcat/openal-soft)
* [assimp](https://github.com/assimp/assimp)

Alternatively, all libraries can be downloaded from a single package [here](https://github.com/Klemmbaustein/Klemmgine/releases/tag/lib)

The paths to both the include paths and library paths to these libraries
must be supplied to the ProjectGenerator.

If you have downloaded the library package, click on "Open library package" and navigate to the extracted package folder.

<img src="Git/Installation.png" width="640" height="480">

Once this is done, Visual Studio project files for the engine can be generated. Press the green "Generate" button.

Now the engine binaries need to be built. In Visual Studio, click `Build`, then `Batch Build`. Then `Select All` and `Build`.

<img src="Git/Build.png" width="400" height="300">
<img src="Git/BatchBuild.png" width="540" height="300">

After the binaries are built, open the launcher again. You can now create a new project or import an existing project.

## GNU make (On linux or with [MSYS2](https://www.msys2.org/))

> TODO
