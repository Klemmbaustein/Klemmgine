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
			std::vector<unsigned int> Indices;
			std::string ElemMaterial;
			void GenerateNormals();
			void RemoveDuplicateVertices();
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

		// Merges all elements into a single one. Returns the reference to the new element.
		Element& MergeAll();

		void SeperateElementToGrid(size_t Index, float GridSize);

		//Combine all meshes of the model to a single mesh
		std::vector<Vertex> GetMergedVertices() const;
		std::vector<unsigned int> GetMergedIndices() const;
	};
}