#pragma once

#include <iostream>
#include "Math/Vector.h"
#include <fstream>
#include "Engine/Utility/FileUtility.h"


struct ImportMesh
{
	std::vector<Vector3> Positions;
	std::vector<Vector3> Normals;
	std::vector<Vector2> UVs;
	std::vector<int> Indicies;
};

extern std::vector<ImportMesh> Meshes;

namespace ModelImporter
{
	std::string Import(std::string Name, std::string CurrentFilepath);
}