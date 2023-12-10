Write-Host "Building dependencies..."

Write-Host "--- Building SDL2 ---"
cd Dependencies\SDL\VisualC\SDL
msbuild -noconsolelogger -nologo /p:Configuration=Release /p:Platform=x64
cd ..\..\..\..
Write-Host "--- Finished building SDL2 ---"

Write-Host "--- Building SDL2_net ---"
cd Dependencies\SDL_net
cmake cmake -S . -B Build -DSDL2_LIBRARY="..\SDL\VisualC\SDL\x64\Release\SDL2.lib" -DSDL2_INCLUDE_DIR="..\SDL\include"
cd Build\
msbuild SDL2_net.vcxproj /p:Configuration=Release /p:Platform=x64
cd ..\..\..
Write-Host "--- Finished SDL2_net ---"

Write-Host "--- Configuring glew ---"
cd Dependencies\glew-cmake
cmake CMakeLists.txt
Write-Host "--- Building glew ---"
msbuild libglew_static.vcxproj -nologo /p:Configuration=Release /p:Platform=x64
cd ..\..
Write-Host "--- Finished building glew ---"

Write-Host "--- Configuring assimp ---"
cd Dependencies\assimp
cmake CMakeLists.txt
Write-Host "--- Building assimp ---"
msbuild code\assimp.vcxproj -nologo /p:Configuration=Release /p:Platform=x64
cd ..\..
Write-Host "--- Finished building assimp ---"

Write-Host "--- Configuring OpenAL ---"
cd Dependencies\openal-soft
cmake CMakeLists.txt
Write-Host "--- Building OpenAL ---"
msbuild OpenAL.vcxproj -nologo /p:Configuration=Release /p:Platform=x64
cd ..\..
Write-Host "--- Finished building OpenAL ---"
Write-Host "--- Finished setting up dependencies ---"

Write-Host "--- Building Engine ---"
dotnet restore
if ($args[0] -eq "CI_BUILD")
{
	$Env:ExternalCompilerOptions = "ENGINE_NO_SOURCE=1"
	msbuild Klemmgine.sln /p:Configuration=Release /p:Platform=x64 /p:CI_BUILD=1
	msbuild Klemmgine.sln /p:Configuration=Editor /p:Platform=x64 /p:CI_BUILD=1
	msbuild Klemmgine.sln /p:Configuration=Debug /p:Platform=x64 /p:CI_BUILD=1
	msbuild Klemmgine.sln /p:Configuration=Server /p:Platform=x64 /p:CI_BUILD=1

	./ProjectGenerator.exe -projectName EngineBuild -includeEngine false -ciBuild true

	cd Games/EngineBuild

	dotnet restore

	ls

	msbuild EngineBuild.sln /p:Configuration=Release /p:Platform=x64 /p:CI_BUILD=1
	msbuild EngineBuild.sln /p:Configuration=Editor /p:Platform=x64 /p:CI_BUILD=1
	msbuild EngineBuild.sln /p:Configuration=Debug /p:Platform=x64 /p:CI_BUILD=1
	msbuild EngineBuild.sln /p:Configuration=Server /p:Platform=x64 /p:CI_BUILD=1
	rm x64 -r -force
	rm GeneratedIncludes -r -force
	rm Code -r -force
	rm bin\*.pdb
	cd ../..

	mkdir Tools/ProjectGenerator/ProjectFilesNoSource/bin/
	cp Games/EngineBuild/bin/* Tools/ProjectGenerator/ProjectFilesNoSource/bin/

	$NativeFiles = "x64/", "lib/", "Dependencies/", "EngineSource/"

	rm $NativeFiles
}
else
{
	Write-Host $args[0]
	msbuild Klemmgine.sln /p:Configuration=Release /p:Platform=x64
}
Write-Host "--- Done ---"