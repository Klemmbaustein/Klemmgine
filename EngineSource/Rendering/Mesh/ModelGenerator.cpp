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
	ModelData::Element& ModelData::AddElement()
	{
		Elements.push_back(Element());
		return Elements[Elements.size() - 1];
	}
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
			std::vector<unsigned int> Indices;
			Input.read((char*)&NewNumVertices, sizeof(int));
			Input.read((char*)&NewNumIndices, sizeof(int));
			// Read vertices

			for (int i = 0; i < NewNumVertices; i++)
			{
				Vertex vertex;
				Input.read((char*)&vertex.Position.x, sizeof(float));
				Input.read((char*)&vertex.Position.y, sizeof(float));
				Input.read((char*)&vertex.Position.z, sizeof(float));
				Input.read((char*)&vertex.Normal.x, sizeof(float));
				Input.read((char*)&vertex.Normal.y, sizeof(float));
				Input.read((char*)&vertex.Normal.z, sizeof(float));
				Input.read((char*)&vertex.TexCoord.x, sizeof(float));
				Input.read((char*)&vertex.TexCoord.y, sizeof(float));
				// Calculate Size for AABB collision box
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
			// Read Indices
			for (int i = 0; i < NewNumIndices; i++)
			{
				int Index = 0;
				Input.read((char*)&Index, sizeof(int));
				Indices.push_back(Index);
			}

			// Read materials
			std::string Name;
			size_t len;
			Input.read((char*)&len, sizeof(size_t));
			char* temp = new char[len + 1];
			Input.read(temp, len);
			temp[len] = '\0';
			Name = temp;
			delete[] temp;

			Elements.push_back(Element(Vertices, Indices, Name));

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

	void ModelData::SaveModelData(std::string Path, bool MaterialsInContent)
	{
		std::ofstream Output(Path, std::ios::out | std::ios::binary);

		int NumElements = Elements.size();
		Output.write((char*)&NumElements, sizeof(int));
		for (int j = 0; j < NumElements; j++)
		{
			int NumVertices = Elements[j].Vertices.size();
			int NumIndices = Elements[j].Indices.size();

			Output.write((char*)&NumVertices, sizeof(int));
			Output.write((char*)&NumIndices, sizeof(int));

			auto& ElemVerts = Elements[j].Vertices;
			auto& ElemInds = Elements[j].Indices;

			for (int i = 0; i < NumVertices; i++)
			{
				Output.write((char*)&ElemVerts[i].Position.x, sizeof(float));
				Output.write((char*)&ElemVerts[i].Position.y, sizeof(float));
				Output.write((char*)&ElemVerts[i].Position.z, sizeof(float));
				Output.write((char*)&ElemVerts[i].Normal.x, sizeof(float));
				Output.write((char*)&ElemVerts[i].Normal.y, sizeof(float));
				Output.write((char*)&ElemVerts[i].Normal.z, sizeof(float));
				Output.write((char*)&ElemVerts[i].TexCoord.x, sizeof(float));
				Output.write((char*)&ElemVerts[i].TexCoord.y, sizeof(float));
			}
			for (int i = 0; i < NumIndices; i++)
			{
				Output.write((char*)&ElemInds[i], sizeof(int));
			}
			std::string MaterialString = MaterialsInContent ? "Content/" + Elements[j].ElemMaterial : Elements[j].ElemMaterial;
			size_t size = MaterialString.size();
			Output.write((char*)&size, sizeof(size_t));
			Output.write(MaterialString.c_str(), sizeof(char) * size);
		}
		Output.write((char*)&CastShadow, sizeof(bool));
		Output.write((char*)&HasCollision, sizeof(uint8_t));
		Output.write((char*)&TwoSided, sizeof(uint8_t));
		Output.close();
	}
	std::vector<Vertex> ModelData::GetMergedVertices() const
	{
		std::vector<Vertex> MergedVertices;
		for (auto& elem : Elements)
		{
			for (size_t i = 0; i < elem.Vertices.size(); i++)
			{
				MergedVertices.push_back(elem.Vertices[i]);
			}
		}
		return MergedVertices;
	}
	std::vector<unsigned int> ModelData::GetMergedIndices() const
	{
		std::vector<unsigned int> MergedIndices;
		size_t prevSize = 0;
		for (size_t i = 0; i < Elements.size(); i++)
		{
			for (size_t j = 0; j < Elements[i].Indices.size(); j++)
			{
				MergedIndices.push_back(Elements[i].Indices[j] + prevSize);
			}
			prevSize += Elements[i].Vertices.size();
		}
		return MergedIndices;
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

	void ModelData::SeperateElementToGrid(size_t Index, float Size)
	{
		MakeCollisionBox();

		for (float X = CollisionBox.minX; X < CollisionBox.maxX; X += Size)
		{
			for (float Y = CollisionBox.minY; Y < CollisionBox.maxY; Y += Size)
			{
				for (float Z = CollisionBox.minZ; Z < CollisionBox.maxY; Z += Size)
				{
					Collision::Box SegmentBox = Collision::Box(X, X + Size, Y, Y + Size, Z, Z + Size);
					ModelData::Element NewElement;
					auto& CurrentElement = Elements[Index];

					size_t NewIndex = 0;

					for (size_t i = 0; i < CurrentElement.Indices.size(); i += 3)
					{
						auto& TriA = CurrentElement.Vertices[CurrentElement.Indices[i]];
						auto& TriB = CurrentElement.Vertices[CurrentElement.Indices[i + 1]];
						auto& TriC = CurrentElement.Vertices[CurrentElement.Indices[i + 2]];

						float Tri2Length = glm::length((TriA.Position - TriB.Position)
							+ (TriA.Position - TriC.Position));

						if (SegmentBox.SphereInBox(TriA.Position, Tri2Length))
						{
							NewElement.Vertices.push_back(TriA);
							NewElement.Vertices.push_back(TriB);
							NewElement.Vertices.push_back(TriC);
							NewElement.Indices.push_back(NewIndex++);
							NewElement.Indices.push_back(NewIndex++);
							NewElement.Indices.push_back(NewIndex++);
						}
					}
					if (!NewElement.Vertices.empty())
					{
						Elements.push_back(NewElement);
					}
				}
			}
		}

		Elements.erase(Elements.begin() + Index);
	}

	void ModelData::Element::GenerateNormals()
	{
		for (size_t i = 0; i < Indices.size(); i += 3) // <- Flat shading
		{
			size_t A = Indices[i], B = Indices[i + 1], C = Indices[i + 2];
			Vector3 n = Vector3::Cross(Vertices[B].Position - Vertices[A].Position, Vertices[C].Position - Vertices[A].Position);
			Vertices[A].Normal += (glm::vec3)n;
			Vertices[B].Normal += (glm::vec3)n;
			Vertices[C].Normal += (glm::vec3)n;
		}
		for (auto& v : Vertices)
		{
			v.Normal = glm::normalize(v.Normal);
		}
	}
	void ModelData::Element::MakeCube(int32_t Resolution, Vector3 Offset)
	{
		Vector3 CubeDirections[6] =
		{
			Vector3(1,  0,  0),
			Vector3(-1,  0,  0),
			Vector3(0,  1,  0),
			Vector3(0, -1,  0),
			Vector3(0,  0,  1),
			Vector3(0,  0, -1)
		};
		for (auto dir : CubeDirections)
		{
			AddFace(Resolution, dir, Offset);
		}
	}

	ModelData::Element& ModelData::MergeAll()
	{
		auto Verts = GetMergedVertices();
		auto Inds = GetMergedIndices();
		Elements.clear();
		auto& NewElem = AddElement();
		NewElem.Vertices = Verts;
		NewElem.Indices = Inds;
		return NewElem;
	}

	void ModelData::Element::AddFace(int32_t Resolution, Vector3 Normal, Vector3 Offset)
	{
		glm::vec3 AxisA = glm::vec3(Normal.Y, Normal.Z, Normal.X);
		glm::vec3 AxisB = glm::cross((glm::vec3)Normal, AxisA);
		size_t InitialSize = this->Vertices.size();
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
				Vertices.push_back(Vertex((Point + Offset) * 200, glm::vec2(texcoords.x, texcoords.y), glm::vec4(1, 1, 1, 1), Normal));
				if (x != Resolution - 1 && y != Resolution - 1)
				{
					this->Indices.push_back(VertexIndex);
					this->Indices.push_back(VertexIndex + Resolution);
					this->Indices.push_back(VertexIndex + Resolution + 1);
					this->Indices.push_back(VertexIndex);
					this->Indices.push_back(VertexIndex + Resolution + 1);
					this->Indices.push_back(VertexIndex + 1);
				}
			}
		}
	}
	void ModelData::Element::Sphereize(float Distance)
	{
		for (auto& v : Vertices)
		{
			v.Position = Vector3(v.Position).Normalize() * Distance;
		}
	}
	void ModelData::Element::Clear()
	{
		Vertices.clear();
		Indices.clear();
		ElemMaterial.clear();
	}
}