Write-Host "Building dependencies..."

Write-Host "--- Building SDL2 ---"
cd Dependencies\SDL\VisualC\SDL
msbuild -noconsolelogger -nologo /p:Configuration=Release /p:Platform=x64 /property:MultiProcessorCompilation=true
cd ..\..\..\..
Write-Host "--- Finished building SDL2 ---"

Write-Host "--- Building SDL2_net ---"
cd Dependencies\SDL_net
cmake cmake -S . -B Build -DSDL2_LIBRARY="..\SDL\VisualC\SDL\x64\Release\SDL2.lib" -DSDL2_INCLUDE_DIR="..\SDL\include"
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
if ($args[0] -eq "CI_BUILD")
{
	$Env:ExternalCompilerOptions = "ENGINE_NO_SOURCE=1"
	msbuild Klemmgine.sln /p:Configuration=Release /p:Platform=x64 /p:CI_BUILD=1
	msbuild Klemmgine.sln /p:Configuration=Editor /p:Platform=x64 /p:CI_BUILD=1
	msbuild Klemmgine.sln /p:Configuration=Debug /p:Platform=x64 /p:CI_BUILD=1
	msbuild Klemmgine.sln /p:Configuration=Server /p:Platform=x64 /p:CI_BUILD=1

	./ProjectGenerator.exe -projectName Klemmgine -includeEngine false -ciBuild true

	cd Games/Klemmgine

	dotnet restore

	ls

	msbuild Klemmgine.sln /p:Configuration=Release /p:Platform=x64 /p:CI_BUILD=1
	msbuild Klemmgine.sln /p:Configuration=Editor /p:Platform=x64 /p:CI_BUILD=1
	msbuild Klemmgine.sln /p:Configuration=Debug /p:Platform=x64 /p:CI_BUILD=1
	msbuild Klemmgine.sln /p:Configuration=Server /p:Platform=x64 /p:CI_BUILD=1
	rm x64 -r -force
	rm GeneratedIncludes -r -force
	rm Code -r -force
	rm bin\*.pdb
	cd ../..

	mkdir Tools/ProjectGenerator/ProjectFilesNoSource/bin/
	cp Games/Klemmgine/bin/* Tools/ProjectGenerator/ProjectFilesNoSource/bin/
	cp Games/Klemmgine/*.dll Tools/ProjectGenerator/ProjectFilesNoSource/

	rm x64/ -r -force
	rm lib/ -r -force
	rm Dependencies/ -r -force
	rm EngineSource/ -r -force
	rm Tools/ProjectGenerator/x64 -r -force
	rm Tools/ProjectGenerator/x64 -r -force
	rm Tools/BuildTool -r -force
	rm Tools/bin -r -force
	rm Tools/ProjectGenerator/ProjectFiles/Code -r -force
	rm Tools/ProjectGenerator/ProjectFilesNoSource/Code -r -force
	rm Games/ -r -force
}
else
{
	Write-Host $args[0]
	msbuild Klemmgine.sln /p:Configuration=Release /p:Platform=x64
}
Write-Host "--- Done ---"