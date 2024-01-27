Write-Host "Building dependencies..."

# Build engine dependencies
Write-Host "--- Building SDL2 ---"
cd Dependencies\SDL\VisualC\SDL
msbuild -noconsolelogger -nologo /p:Configuration=Release /p:Platform=x64 /property:MultiProcessorCompilation=true
cd ..\..\..\..
Write-Host "--- Finished building SDL2 ---"

Write-Host "--- Building SDL2_net ---"
cd Dependencies\SDL_net
cmake -S . -B Build -DSDL2_LIBRARY="..\SDL\VisualC\SDL\x64\Release\SDL2.lib" -DSDL2_INCLUDE_DIR="..\SDL\include"
cd Build\
msbuild SDL2_net.vcxproj /p:Configuration=Release /p:Platform=x64 /property:MultiProcessorCompilation=true
cd ..\..\..
Write-Host "--- Finished SDL2_net ---"

Write-Host "--- Configuring glew ---"
cd Dependencies\glew-cmake
cmake CMakeLists.txt
Write-Host "--- Building glew ---"
msbuild libglew_static.vcxproj -nologo /p:Configuration=Release /p:Platform=x64 /property:MultiProcessorCompilation=true
cd ..\..
Write-Host "--- Finished building glew ---"

Write-Host "--- Configuring assimp ---"
cd Dependencies\assimp
cmake CMakeLists.txt
Write-Host "--- Building assimp ---"
msbuild code\assimp.vcxproj -nologo /p:Configuration=Release /p:Platform=x64 /property:MultiProcessorCompilation=true
cd ..\..
Write-Host "--- Finished building assimp ---"

Write-Host "--- Configuring OpenAL ---"
cd Dependencies\openal-soft
cmake CMakeLists.txt
Write-Host "--- Building OpenAL ---"
msbuild OpenAL.vcxproj -nologo /p:Configuration=Release /p:Platform=x64 /property:MultiProcessorCompilation=true
cd ..\..
Write-Host "--- Finished building OpenAL ---"
Write-Host "--- Finished setting up dependencies ---"

Write-Host "--- Building Engine ---"
dotnet restore

#if the argument CI_BUILD has been passed through the command line, build the engine for a continuous integration build.
if ($args[0] -eq "CI_BUILD")
{
	function Build-KlemmgineProject 
	{
		Param($config, $name)
		msbuild $name /p:Configuration=$Configuration /p:Platform=x64 /p:CI_BUILD=1
	}

	$engine_configs = 
	@(
		"Release",
		"Editor",
		"Debug",
		"Server"
	)

	Write-Host "--- Building for CI ---"
	# Build the engine
	$Env:ExternalCompilerOptions = "ENGINE_NO_SOURCE=1"

	foreach ($config in $engine_configs)
	{
		Build-KlemmgineProject -name Klemmgine.sln -config $config
	}

	./ProjectGenerator.exe -projectName Klemmgine -includeEngine false -ciBuild true
	cd Games/Klemmgine
	dotnet restore

	foreach ($config in $engine_configs)
	{
		Build-KlemmgineProject -name Klemmgine.sln -config $config
	}
	rm x64 -r -force
	rm GeneratedIncludes -r -force
	rm Code -r -force
	rm bin\*.pdb
	cd ../..

	# Create the directory for project files for "Without Source" builds (CI Builds)
	mkdir Tools/ProjectGenerator/ProjectFilesNoSource/bin/
	cp Games/Klemmgine/bin/* Tools/ProjectGenerator/ProjectFilesNoSource/bin/
	cp Games/Klemmgine/*.dll Tools/ProjectGenerator/ProjectFilesNoSource/

	# Download/Install doxygen.
	Invoke-WebRequest https://www.doxygen.nl/files/doxygen-1.10.0.windows.x64.bin.zip -OutFile doxygen.zip

	Expand-Archive doxygen.zip

	# Run doxygen on the engine, output will be in ./Docs/html
	./doxygen/doxygen

	# Remove unused files
	$unused_files = 
	@(
		"doxygen.zip",
		"doxygen/",
		"x64/",
		"Dependencies/",
		"lib/",
		"EngineSource/",
		"Tools/ProjectGenerator/x64/",
		"Tools/BuildTool/",
		"Tools/bin/",
		"Tools/ProjectGenerator/ProjectFiles/Code/",
		"Tools/ProjectGenerator/ProjectFilesNoSource/Code/",
		"Games/"
	)

	foreach ($file in $unused_files) 
	{
	#	rm -r -fo $file
	}
}
else
{
	msbuild Klemmgine.sln /p:Configuration=Release /p:Platform=x64
}
Write-Host "--- Done ---"