#include "ModelGenerator.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <World/Assets.h>
#include <Engine/Log.h>
#include <glm/ext/vector_float2.hpp>
#include <glm/geometric.hpp>


namespace ModelGenerator
{
	void ModelData::LoadModelFromFile(std::string Path)
	{
		if (!std::filesystem::exists(Path))
		{
			std::string PrevPath = Path;
			Path = Assets::GetAsset(Path + ".jsm");
			if (!std::filesystem::exists(Path))
			{
				Log::Print("Could not load model: " + PrevPath, Log::LogColor::Red);
				return;
			}
		}
		std::ifstream Input = std::ifstream(Path, std::ios::in | std::ios::binary);

		uint32_t NumMeshes;
		Input.read((char*)&NumMeshes, sizeof(int));

		int NewNumVertices; int NewNumIndices;
		for (int j = 0; j < NumMeshes; j++)
		{
			std::vector<Vertex> Vertices;
			std::vector<int> Indices;
			Input.read((char*)&NewNumVertices, sizeof(int));
			Input.read((char*)&NewNumIndices, sizeof(int));
			//read verts

			for (int i = 0; i < NewNumVertices; i++)
			{
				Vertex vertex;
				Input.read((char*)&vertex.Position.x, sizeof(float));
				Input.read((char*)&vertex.Position.y, sizeof(float));
				Input.read((char*)&vertex.Position.z, sizeof(float));
				Input.read((char*)&vertex.Normal.x, sizeof(float));
				Input.read((char*)&vertex.Normal.y, sizeof(float));
				Input.read((char*)&vertex.Normal.z, sizeof(float));
				Input.read((char*)&vertex.U, sizeof(float));
				Input.read((char*)&vertex.V, sizeof(float));
				//Calculate Size for AABB collision box
				if (vertex.Position.x > CollisionBox.maxX)
				{
					CollisionBox.maxX = vertex.Position.x;
				}
				if (vertex.Position.y > CollisionBox.maxY)
				{
					CollisionBox.maxY = vertex.Position.y;
				}
				if (vertex.Position.z > CollisionBox.maxZ)
				{
					CollisionBox.maxZ = vertex.Position.z;
				}
				if (vertex.Position.x < CollisionBox.minX)
				{
					CollisionBox.minX = vertex.Position.x;
				}
				if (vertex.Position.y < CollisionBox.minY)
				{
					CollisionBox.minY = vertex.Position.y;
				}
				if (vertex.Position.z < CollisionBox.minZ)
				{
					CollisionBox.minZ = vertex.Position.z;
				}
				Vertices.push_back(vertex);
			}
			//Read Indices


			for (int i = 0; i < NewNumIndices; i++)
			{
				int Index = 0;
				Input.read((char*)&Index, sizeof(int));
				Indices.push_back(Index);
			}

			//read materials
			std::string Name;
			this->Vertices.push_back(Vertices);
			this->Indices.push_back(Indices);
			size_t len;
			Input.read((char*)&len, sizeof(size_t));
			char* temp = new char[len + 1];
			Input.read(temp, len);
			temp[len] = '\0';
			Name = temp;
			delete[] temp;
			Materials.push_back(Name);
		}
		Input.read((char*)&CastShadow, sizeof(uint8_t));
		try
		{
			Input.read((char*)&HasCollision, sizeof(uint8_t));
			Input.read((char*)&TwoSided, sizeof(uint8_t));
		}
		catch (std::exception) {}
		Input.close();

		return;
	}

	void ModelData::Clear()
	{
		*this = ModelGenerator::ModelData();
	}

	void ModelData::GenerateNormals(int Index)
	{
		for (size_t i = 0; i < Indices[Index].size(); i += 3) // <- Flat shading
		{
			size_t A = Indices[Index][i], B = Indices[Index][i + 1], C = Indices[Index][i + 2];
			Vector3 n = Vector3::Cross(Vertices[Index][B].Position - Vertices[Index][A].Position, Vertices[Index][C].Position - Vertices[Index][A].Position);
			Vertices[Index][A].Normal += (glm::vec3)n;
			Vertices[Index][B].Normal += (glm::vec3)n;
			Vertices[Index][C].Normal += (glm::vec3)n;
		}
		for (auto& v : Vertices[Index])
		{
			v.Normal = glm::normalize(v.Normal);
		}
	}

	void ModelData::SaveModelData(std::string Path, bool MaterialsInContent)
	{
		std::ofstream Output(Path, std::ios::out | std::ios::binary);

		int NumMaterials = Materials.size();
		Output.write((char*)&NumMaterials, sizeof(int));
		for (int j = 0; j < NumMaterials; j++)
		{
			int NumVertices = Vertices[j].size();
			int NumIndices = Indices[j].size();

			Output.write((char*)&NumVertices, sizeof(int));
			Output.write((char*)&NumIndices, sizeof(int));

			for (int i = 0; i < NumVertices; i++)
			{
				Output.write((char*)&Vertices[j][i].Position.x, sizeof(float));
				Output.write((char*)&Vertices[j][i].Position.y, sizeof(float));
				Output.write((char*)&Vertices[j][i].Position.z, sizeof(float));
				Output.write((char*)&Vertices[j][i].Normal.x, sizeof(float));
				Output.write((char*)&Vertices[j][i].Normal.y, sizeof(float));
				Output.write((char*)&Vertices[j][i].Normal.z, sizeof(float));
				Output.write((char*)&Vertices[j][i].U, sizeof(float));
				Output.write((char*)&Vertices[j][i].V, sizeof(float));
			}
			for (int i = 0; i < NumIndices; i++)
			{
				Output.write((char*)&Indices[j][i], sizeof(int));
			}
			std::string MaterialString = MaterialsInContent ? "Content/" + Materials[j] : Materials[j];
			size_t size = MaterialString.size();

			Output.write((char*)&size, sizeof(size_t));
			std::string OutPath = MaterialString;

			Output.write(OutPath.c_str(), sizeof(char) * size);
		}
		Output.write((char*)&CastShadow, sizeof(bool));
		Output.write((char*)&HasCollision, sizeof(uint8_t));
		Output.write((char*)&TwoSided, sizeof(uint8_t));
		Output.close();
	}
	std::vector<Vertex> ModelData::GetMergedVertices()
	{
		std::vector<Vertex> MergedVertices;
		for (size_t i = 0; i < Vertices.size(); i++)
		{
			for (size_t j = 0; j < Vertices[i].size(); j++)
			{
				MergedVertices.push_back(Vertices[i][j]);
			}
		}
		return MergedVertices;
	}
	std::vector<int> ModelData::GetMergedIndices()
	{
		std::vector<int> MergedIndics;
		size_t prevSize = 0;
		for (size_t i = 0; i < Indices.size(); i++)
		{
			for (size_t j = 0; j < Indices[i].size(); j++)
			{
				MergedIndics.push_back(Indices[i][j] + prevSize);
			}
			prevSize += Vertices[i].size();
		}
		return MergedIndics;
	}

	void ModelData::Sphereize(float Distance, int Index)
	{
		if (Index == -1)
		{
			for (int i = 0; i < Vertices.size(); i++)
			{
				Sphereize(Distance, i);
			}
		}
		else if(Index >= 0 && Index < Vertices.size())
		{
			for (auto& v : Vertices[Index])
			{
				v.Position = Vector3(v.Position).Normalize() * Distance;
			}
		}
		else
		{
			Log::Print("Invalid mesh index passed to 'Sphereize' function", Vector3(1, 0, 0));
		}
	}
	void ModelData::MakeCube(int32_t Resolution, int Index, Vector3 Offset)
	{
		Vector3 CubeDirections[6] =
		{
			Vector3( 1,  0,  0),
			Vector3(-1,  0,  0),
			Vector3( 0,  1,  0),
			Vector3( 0, -1,  0),	
			Vector3( 0,  0,  1),
			Vector3( 0,  0, -1)
		};
		for (auto dir : CubeDirections)
		{
			AddFace(Resolution, Index, dir, Offset);
		}
		MakeCollisionBox();
	}
	void ModelData::AddFace(int32_t Resolution, int Index, Vector3 Normal, Vector3 Offset)
	{
		if (Index >= Vertices.size())
		{
			Vertices.resize(Index + 1);
			Indices.resize(Index + 1);
			Materials.resize(Index + 1);
		}
		glm::vec3 AxisA = glm::vec3(Normal.Y, Normal.Z, Normal.X);
		glm::vec3 AxisB = glm::cross((glm::vec3)Normal, AxisA);
		size_t InitialSize = this->Vertices[Index].size();
		for (int32_t x = 0; x < Resolution; x++)
		{
			for (int32_t y = 0; y < Resolution; y++)
			{
				int VertexIndex = x + y * Resolution + InitialSize;
				glm::vec2 t = glm::vec2(x, y) / (Resolution - 1.f);
				glm::vec3 Point = (Normal + AxisA * (2 * t.x - 1) + AxisB * (2 * t.y - 1)) / glm::vec3(2);
				glm::vec2 texcoords = glm::vec2(0);
				if ((Normal) == glm::vec3(0, 1, 0))
				{
					texcoords = t;
				}
				else if (Normal == glm::vec3(0, -1, 0))
				{
					texcoords = t;
				}
				else
				{
					glm::vec3 Pos = AxisA * (2 * t.x - 1) + AxisB * (2 * t.y - 1);
					Pos = Pos / glm::vec3(2, -2, 2);
					Pos = Pos - glm::vec3(-0.5, -0.5, 0);
					texcoords = glm::vec2(Pos.x + Pos.z, Pos.y);
				}
				Vertices[Index].push_back(Vertex((Point + Offset) * 200, texcoords.x, texcoords.y, 1, 1, 1, 1, Normal));
				if (x != Resolution - 1 && y != Resolution - 1)
				{
					this->Indices[Index].push_back(VertexIndex);
					this->Indices[Index].push_back(VertexIndex + Resolution);
					this->Indices[Index].push_back(VertexIndex + Resolution + 1);
					this->Indices[Index].push_back(VertexIndex);
					this->Indices[Index].push_back(VertexIndex + Resolution + 1);
					this->Indices[Index].push_back(VertexIndex + 1);
				}
			}
		}
	}
	void ModelData::MakeCollisionBox()
	{
		for (auto& i : GetMergedVertices())
		{
			if (i.Position.x > CollisionBox.maxX)
			{
				CollisionBox.maxX = i.Position.x;
			}
			if (i.Position.y > CollisionBox.maxY)
			{
				CollisionBox.maxY = i.Position.y;
			}
			if (i.Position.z > CollisionBox.maxZ)
			{
				CollisionBox.maxZ = i.Position.z;
			}
			if (i.Position.x < CollisionBox.minX)
			{
				CollisionBox.minX = i.Position.x;
			}
			if (i.Position.y < CollisionBox.minY)
			{
				CollisionBox.minY = i.Position.y;
			}
			if (i.Position.z < CollisionBox.minZ)
			{
				CollisionBox.minZ = i.Position.z;
			}
		}
	}
}