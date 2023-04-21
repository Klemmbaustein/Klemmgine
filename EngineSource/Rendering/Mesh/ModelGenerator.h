#pragma once
#include <string>
#include <vector>
#include <Rendering/Vertex.h>
#include <Math/Collision/CollisionBox.h>

namespace ModelGenerator
{
	struct ModelData
	{
		struct Element
		{
			std::vector<Vertex> Vertices;
			std::vector<int> Indices;
			std::string ElemMaterial;
			void GenerateNormals();
			void MakeCube(int32_t Resolution, Vector3 Offset);
			void AddFace(int32_t Resolution, Vector3 Normal, Vector3 Offset);

			//Convert to a sphere by normalizing all points, then multiplying them by 'distance'
			void Sphereize(float Distance);

			void Clear();

		};
		std::vector<Element> Elements;
		Collision::Box CollisionBox;

		Element& AddElement();

		bool CastShadow = true, TwoSided = false, HasCollision = false;
		//Load a .jsm file, add it to the geometry of the model
		void LoadModelFromFile(std::string File);

		void Clear();

		//Save the generated model to a file
		void SaveModelData(std::string File, bool MaterialsInContent = true);

		void MakeCollisionBox();

		//Combine all meshes of the model to a single mesh
		std::vector<Vertex> GetMergedVertices();
		std::vector<int> GetMergedIndices();
	};
}