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
cd ../JoltPhysics/Build
./cmake_linux_clang_gcc.sh Distribution c++ -DINTERPROCEDURAL_OPTIMIZATION=OFF
cd Linux_Distribution
make -j 4
cd  ../../../..

if [ "$is_ci" != 1 ]; then
	KlemmBuild engine.kbld -DGenerator || fail
else
	KlemmBuild engine.kbld -DGenerator -DCiBuild || fail
fi
declare -a cs_projects=("CSharp/Engine/KlemmgineCSharp.csproj" "CSharp/Core/CSharpCore.csproj")

for i in "${cs_projects[@]}"; do
	dotnet build $i || fail
done

if [ "$is_ci" != 1 ]; then
	echo "Built dependencies and project generator."
	exit 0
fi

echo "Building for CI"

./ProjectGenerator -projectName Klemmgine -includeEngine false -ciBuild true || fail

cd Games/Klemmgine
declare -a configs=("-DEditor" "-DDebug" "-DRelease" "-DServer")

for i in "${configs[@]}"; do
	KlemmBuild makefile.kbld $i -DCiBuild || fail
done

mkdir -p ../../Tools/ProjectGenerator/ProjectFilesNoSource/bin
cp ./bin/* ../../Tools/ProjectGenerator/ProjectFilesNoSource/bin
rm -rf Build
