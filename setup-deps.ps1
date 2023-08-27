Write-Host "Building dependencies..."

Write-Host "--- Building SDL2 ---"
cd Dependencies\SDL\VisualC\SDL
msbuild -noconsolelogger -nologo /p:Configuration=Release /p:Platform=x64
cd ..\..\..\..
Write-Host "--- Finished building SDL2 ---"

Write-Host "--- Configuring glew ---"
cd Dependencies\glew-cmake
cmake -S . -B Build/
Write-Host "--- Building glew ---"
msbuild Build\libglew_static.vcxproj -nologo /p:Configuration=Release /p:Platform=x64
cd ..\..
Write-Host "--- Finished building glew ---"

Write-Host "--- Configuring assimp ---"
cd Dependencies\assimp
cmake -S . -B Build/
Write-Host "--- Building assimp ---"
msbuild Build\assimp.vcxproj -nologo /p:Configuration=Release /p:Platform=x64
cd ..\..
Write-Host "--- Finished building assimp ---"

Write-Host "--- Configuring OpenAL ---"
cd Dependencies\openal-soft
cmake CMakeLists.txt -S . -B Build/
Write-Host "--- Building OpenAL ---"
msbuild Build/OpenAL.vcxproj -nologo /p:Configuration=Release /p:Platform=x64
cd ..\..
Write-Host "--- Finished building OpenAL ---"
Write-Host "--- Finished setting up dependencies ---"