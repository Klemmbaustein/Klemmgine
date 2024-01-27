#!/usr/bin/bash

for i in "$*" ; do
	if [[ $i == "CI_BUILD" ]] ; then
 		echo "Building for CI"
		is_ci=1
		break
	fi
done

fail() 
{
	echo "Setup failed - exiting..."
	exit 1
}

cd Dependencies/SDL_net
cmake -S . -B Build/ || fail
cd Build
make -j 4 || fail
cd ../../assimp/
cmake CMakeLists.txt || fail
make -j 4 || fail

cd ../..

KlemmBuild engine.kbld -DGenerator -DCiBuild || fail

if [ "$is_ci" != 1 ]; then
	echo "Built dependencies and project generator."
	exit 0
fi

echo "Building for CI"

./ProjectGenerator -projectName Klemmgine -includeEngine false -ciBuild true || fail

declare -a cs_projects=("CSharp/Engine/KlemmgineCSharp.csproj" "CSharp/Core/CSharpCore.csproj")

for i in "${cs_projects[@]}"; do
	dotnet build $i || fail
done

cd Games/Klemmgine
declare -a configs=("-DEditor" "-DDebug" "-DRelease" "-DServer")

for i in "${configs[@]}"; do
	KlemmBuild makefile.kbld $i -DCiBuild || fail
done

mkdir -p ../../Tools/ProjectGenerator/ProjectFilesNoSource/bin
cp ./bin/* ../../Tools/ProjectGenerator/ProjectFilesNoSource/bin
rm -rf Build
