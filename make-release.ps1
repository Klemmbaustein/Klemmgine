$ReleaseFolder = "EngineRelease"

if (Test-Path $ReleaseFolder) {
	Remove-Item $ReleaseFolder -Recurse -Force
}

$WithCSharp = 0

if ($args[0] -eq "-csharp") {
	$WithCSharp = 1
	Write-Host "With C#!"
}

$RelevantFiles = "EngineSource/","Tools/","EditorContent","Launcher/","SDL2.dll","ProjectGenerator.exe"
if ($WithCSharp) {
	$RelevantFiles += "CSharpCore/"
}

foreach ($file in $RelevantFiles) {
	Copy-Item -Path $file -Destination "$ReleaseFolder/$file" -Recurse
}

$IrreleventFiles = "Tools/ProjectGenerator/Release/","Tools/ProjectGenerator/x64/"
$IrreleventFiles += "Tools/ProjectGenerator/ProjectGenerator.vcxproj","Tools/ProjectGenerator/ProjectGenerator.vcxproj.filters","Tools/ProjectGenerator/ProjectGenerator.vcxproj.user"
$IrreleventFiles += "Tools/BuildTool/x64","Tools/BuildTool/Release"
$IrreleventFiles += "Tools/BuildTool/BuildTool.vcxproj","Tools/BuildTool/BuildTool.vcxproj.filters","Tools/BuildTool/BuildTool.vcxproj.user"
$IrreleventFiles += "EditorContent/Config/"
if ($WithCSharp) {
	$IrreleventFiles += "CSharpCore/obj","CSharpCore/Build"
} else {
	$IrreleventFiles += "EngineSource/Objects/CSharpObject.h","EngineSource/Objects/CSharpObject.cpp","Tools/ProjectGenerator/DefaultProjectFiles/Scripts/"
}

foreach ($file in $IrreleventFiles) {
	Write-Host $file
	Remove-Item "$ReleaseFolder/$file" -Recurse -Force
}