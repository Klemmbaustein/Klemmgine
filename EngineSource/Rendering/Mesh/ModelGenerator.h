#pragma once
#include <string>
#include <vector>
#include <Rendering/Vertex.h>
#include <Math/Collision/CollisionBox.h>

namespace ModelGenerator
{
	/**
	* @brief
	* Defines a model, which is a collection of polygon meshes.
	*/
	struct ModelData
	{
		/**
		* @brief
		* Represents a polygon mesh in a ModelData struct.
		*/
		struct Element
		{
			/// The mesh vertices.
			std::vector<Vertex> Vertices;
			/// The mesh indices.
			std::vector<unsigned int> Indices;
			/// Name of the material used by the mesh.
			std::string ElemMaterial;
			/// Generates normals for each polygon using their positions.
			void GenerateNormals();
			/**
			* @brief
			* Attempts to remove duplicate vertices from this mesh, adjusts the indices.
			*/
			void RemoveDuplicateVertices();

			/**
			* @brief
			* Creates a cube mesh with the given resolution, at the given point.
			* 
			* @param Resolution
			* The amount of vertices on each edge of the cube.
			* @param Center
			* The point center of the created cube.
			*/
			void MakeCube(int32_t Resolution, Vector3 Center);
			void AddFace(int32_t Resolution, Vector3 Normal, Vector3 Offset);

			/// Converts this mesh to a sphere by normalizing all points, then multiplying them by 'distance'
			void Sphereize(float Distance);

			/// Clears this element of all mesh data.
			void Clear();
		};

		std::vector<Element> Elements;
		Collision::Box CollisionBox;

		/**
		* @brief
		* Adds an element to the ModelData.
		* 
		* @return
		* A reference to the new element.
		*/
		Element& AddElement();

		bool CastShadow = true, TwoSided = false, HasCollision = false;
		/// Loads a model (.jsm) file with the given name, then adds the model data of that file to the current model.
		void LoadModelFromFile(std::string File);

		void Clear();

		/// Saves this model data 
		void SaveModelData(std::string File);

		/**
		* @brief
		* Generates a collision AABB box for this model data.
		*/
		void MakeCollisionBox();

		/**
		* @brief
		* Merges all elements into a single one.
		* @return
		* A reference to the new element.
		*/
		Element& MergeAll();

		void SeperateElementToGrid(size_t Index, float GridSize);

		/**
		* @brief
		* Combines the meshes all elements of this model into a single one. This returns the vertices for that mesh.
		*
		* See also: @ref GetMergedVertices()
		*/
		std::vector<Vertex> GetMergedVertices() const;

		/**
		* @brief
		* Combines the meshes all elements of this model into a single one. This returns the indices for that mesh.
		* 
		* See also: @ref GetMergedIndices()
		*/
		std::vector<unsigned int> GetMergedIndices() const;
	};
}