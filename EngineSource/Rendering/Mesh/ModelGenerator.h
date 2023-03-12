#pragma once
#include <string>
#include <vector>
#include <Rendering/Vertex.h>
#include <Math/Collision/CollisionBox.h>
namespace ModelGenerator
{
	struct ModelData
	{
		std::vector<std::vector<Vertex>> Vertices;
		std::vector<std::vector<int>> Indices;
		std::vector<std::string> Materials;
		Collision::Box CollisionBox;
		bool CastShadow = true, TwoSided = false, HasCollision = false;
		//Load a .jsm file, add it to the geometry of the model
		void LoadModelFromFile(std::string File);

		void Clear();
		void ClearMesh(unsigned int Index);

		void GenerateNormals(int Index = -1);

		//Save the generated model to a file
		void SaveModelData(std::string File, bool MaterialsInContent = true);

		//Convert to a sphere by normalizing all points, then multiplying them by 'distance'
		//Index: the mesh of the model that should be 'Sphereized', -1 = all
		void Sphereize(float Distance, int Index = -1);

		void MakeCube(int32_t Resolution, int Index, Vector3 Offset);
		void AddFace(int32_t Resolution, int Index, Vector3 Normal, Vector3 Offset);
		void MakeCollisionBox();

		//Combine all meshes of the model to a single mesh
		std::vector<Vertex> GetMergedVertices();
		std::vector<int> GetMergedIndices();
	};
}