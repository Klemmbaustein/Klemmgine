#include "BakedLighting.h"
#include <GL/glew.h>
#include <Math/Vector.h>
#include <Engine/EngineRandom.h>
#include <Engine/EngineError.h>
#include <iostream>
#include <Math/Collision/Collision.h>
#include <Rendering/Graphics.h>
#include <Objects/Components/MeshComponent.h>
#include <Rendering/Mesh/ModelGenerator.h>
#include <thread>
#include <Engine/Log.h>
#include <Engine/File/Assets.h>
#include <filesystem>
#include <Engine/Scene.h>
#include <fstream>
#include <Engine/Application.h>
#include <Rendering/Utility/Framebuffer.h>

unsigned int BakedLighting::LightTexture = 0;
float BakedLighting::LightmapScaleMultiplier = 1;
std::atomic<bool> BakedLighting::FinishedBaking = false;
uint64_t BakedLighting::LightmapResolution = 100;
constexpr uint8_t NUM_CHANNELS = 1;
bool BakedLighting::LoadedLightmap = false;

Vector3 LightmapScale = 200.0f;

void BakedLighting::Init()
{
	std::byte* Texture = new std::byte[LightmapResolution * LightmapResolution * LightmapResolution * NUM_CHANNELS]();

	for (uint64_t i = 0; i < LightmapResolution * LightmapResolution * LightmapResolution * NUM_CHANNELS; i += NUM_CHANNELS)
	{
		for (uint8_t c = 0; c < NUM_CHANNELS; c++)
		{
			Texture[i + c] = std::byte(255);
		}
	}

	if (LightTexture)
	{
		glDeleteTextures(1, &LightTexture);
	}
	LoadedLightmap = false;
	glGenTextures(1, &LightTexture);
	glBindTexture(GL_TEXTURE_3D, LightTexture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_3D,
		0,
		GL_R8, 
		(GLsizei)LightmapResolution,
		(GLsizei)LightmapResolution,
		(GLsizei)LightmapResolution,
		0, 
		GL_RED, 
		GL_UNSIGNED_BYTE, 
		Texture);
	delete[] Texture;
	
}

void BakedLighting::BindToTexture()
{
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, LightTexture);
}

int64_t BakedLighting::GetLightTextureSize()
{
	return LightmapResolution;
}

constexpr uint8_t BKDAT_FILE_VERSION = 1;

#if EDITOR

namespace Bake
{
	struct BakeMesh
	{
		ModelGenerator::ModelData MeshData;
		Transform MeshTransform;
	};

	std::vector<BakeMesh> Meshes;
	std::vector<Graphics::Light> Lights;
	Vector3 BakeScale;

	static Vector3 BakeMapToPos(uint64_t TextureElement)
	{
		int64_t x = TextureElement % BakedLighting::LightmapResolution;
		int64_t y = (TextureElement / BakedLighting::LightmapResolution) % BakedLighting::LightmapResolution;
		int64_t z = TextureElement / (BakedLighting::LightmapResolution * BakedLighting::LightmapResolution);

		return Vector3((float)x, (float)y, (float)z) - ((float)BakedLighting::LightmapResolution / 2.0f);
	}


	std::byte* Texture = nullptr;

	constexpr int NUM_CHUNK_SPLITS = 2;
	std::atomic<float> ThreadProgress[NUM_CHUNK_SPLITS * NUM_CHUNK_SPLITS * NUM_CHUNK_SPLITS];

	static void BakeSection(int64_t x, int64_t y, int64_t z, size_t ThreadID)
	{
		const uint64_t SECTION_SIZE = BakedLighting::LightmapResolution / NUM_CHUNK_SPLITS;
		const float ProgressPerPixel = 1.0f / (SECTION_SIZE * SECTION_SIZE * SECTION_SIZE);

		const float TexelSize = BakeScale.X / BakedLighting::LightmapResolution;

		for (uint64_t i = 0; i < BakedLighting::LightmapResolution * BakedLighting::LightmapResolution * BakedLighting::LightmapResolution; i++)
		{
			int64_t px = i % BakedLighting::LightmapResolution;
			int64_t py = (i / BakedLighting::LightmapResolution) % BakedLighting::LightmapResolution;
			int64_t pz = i / (BakedLighting::LightmapResolution * BakedLighting::LightmapResolution);
			px /= SECTION_SIZE;
			py /= SECTION_SIZE;
			pz /= SECTION_SIZE;

			if (px != x || py != y || pz != z)
			{
				continue;
			}


			Vector3 Pos = BakeMapToPos(i);
			Pos = Pos / (float)BakedLighting::LightmapResolution;
			Pos = Pos * BakeScale;
			float Intensity = BakedLighting::GetLightIntensityAt((int64_t)Pos.X, (int64_t)Pos.Y, (int64_t)Pos.Z, TexelSize);
			Texture[i] = std::byte(Intensity * 255);
			ThreadProgress[ThreadID] += ProgressPerPixel;
		}
	}
}

namespace Bake
{
	glm::vec3 SunDirection = glm::vec3(0, -1, 0);

	static inline Collision::HitResponse BakeRayTrace(const glm::vec3& orig, const glm::vec3& end, const glm::vec3& A, const glm::vec3& B, const glm::vec3& C)
	{
		glm::vec3 E1 = C - A;
		glm::vec3 E2 = B - A;
		glm::vec3 N = glm::cross(E1, E2);
		float det = -glm::dot(end, N);
		float invdet = 1.0f / det;
		glm::vec3 AO = orig - A;
		glm::vec3 DAO = glm::cross(AO, end);
		float u = dot(E2, DAO) * invdet;
		float v = -dot(E1, DAO) * invdet;
		float t = dot(AO, N) * invdet;
		if ((t >= 0.0f && u >= 0.0f && v >= 0.0f && (u + v) <= 1.0f))
			return Collision::HitResponse(true, orig + end * t, normalize(N), t);
		else return Collision::HitResponse();
	}

	const float ShadowBias = 2;

	static Collision::HitResponse BakeLine(const glm::vec3& start, const glm::vec3& end, bool mode)
	{
		Collision::HitResponse r;
		if (mode)
		{
			r.Distance = INFINITY;
		}
		else
		{
			r.Distance = 0;
		}
		
		for (auto& mesh : Bake::Meshes)
		{
			Collision::Box BroadPhaseBox = mesh.MeshData.CollisionBox.TransformBy(Transform(mesh.MeshTransform.Position, Vector3(), Vector3(1)));
			if (!Collision::LineCheckForAABB(BroadPhaseBox,
				start, end).Hit)
			{
				continue;
			}
			for (auto& elem : mesh.MeshData.Elements)
			{
				for (size_t i = 0; i < elem.Indices.size(); i += 3)
				{
					glm::vec3* CurrentTriangle[3] =
					{
						&elem.Vertices[elem.Indices[i]].Position,
						&elem.Vertices[elem.Indices[i + 2]].Position,
						&elem.Vertices[elem.Indices[i + 1]].Position
					};
					Collision::HitResponse newR
						= (Bake::BakeRayTrace(start, end - start, *CurrentTriangle[0], *CurrentTriangle[1], *CurrentTriangle[2]));
					if (newR.Hit)
					{
						if (mode && newR.Distance < r.Distance)
						{
							r = newR;
						}
						else if (!mode && newR.Distance > r.Distance)
						{
							r = newR;
						}
					}
				}
			}
		}
		return r;
	}
}

float BakedLighting::GetLightIntensityAt(int64_t x, int64_t y, int64_t z, float ElemSize)
{
	const float TraceDistance = 2500;
	glm::vec3 StartPos = glm::vec3((float)x, (float)y, (float)z);
	StartPos = StartPos + glm::vec3(Bake::BakeScale / (float)LightmapResolution / 2);
	Collision::HitResponse r;
	r.Distance = 0;
	
	float TotalLightIntensity = 0;
	
	for (auto& i : Bake::Lights)
	{
		glm::vec3 pointLightDir = (i.Position - StartPos);
		float dist = length(pointLightDir);
		float NewIntensity = std::pow(std::max((i.Falloff * 10.0f) - dist, 0.0f) / (i.Falloff * 10.0f), 16.0f) * i.Intensity * 16.0f;
		if (NewIntensity > 0)
		{
			r = Bake::BakeLine(i.Position, StartPos, true);
			if (r.Distance && r.Distance < 0.9f)
			{
				NewIntensity = 0;
			}
		}
		TotalLightIntensity += glm::clamp(NewIntensity, 0.0f, 4.0f);
	}
	float LightInt = 0;
	for (int i = 0; i < 1; i++)
	{
		r = Bake::BakeLine(StartPos, StartPos
			+ (Bake::SunDirection
				+ glm::vec3(
					std::fmod((float)std::rand() / 100.0f, 2.0f) - 1.0f,
					std::fmod((float)std::rand() / 100.0f, 2.0f) - 1.0f,
					std::fmod((float)std::rand() / 100.0f, 2.0f) - 1.0f)
				* 0.035f)
			* TraceDistance, false);
		LightInt += r.Hit ? 1 - std::min(r.Distance * TraceDistance / 12.0f, 1.0f) : 1;
	}
	LightInt /= 1.0f;

	return std::min((LightInt / 2.0f + TotalLightIntensity / 4.0f), 1.0f);
	
	return 0;
}

static std::byte Sample3DArray(std::byte* Arr, int64_t x, int64_t y, int64_t z)
{
	if (x < 0 || x >= (int64_t)BakedLighting::LightmapResolution
		|| y < 0 || y >= (int64_t)BakedLighting::LightmapResolution
		|| z < 0 || z >= (int64_t)BakedLighting::LightmapResolution)
	{
		return std::byte(128);
	}

	return Bake::Texture[x * BakedLighting::LightmapResolution * BakedLighting::LightmapResolution + y * BakedLighting::LightmapResolution + z];
}

void BakedLighting::BakeCurrentSceneToFile()
{
	const int Number = 25;
	const bool IsEven = !(Number & 1);
	Bake::Meshes.clear();

	for (WorldObject* i : Objects::AllObjects)
	{
		for (Component* c : i->GetComponents())
		{
			if (!dynamic_cast<MeshComponent*>(c))
			{
				continue;
			}
			if (!dynamic_cast<MeshComponent*>(c)->GetModel()->CastShadow)
			{
				continue;
			}
			Bake::BakeMesh NewMesh;
			NewMesh.MeshData = dynamic_cast<MeshComponent*>(c)->GetModelData();
			if (!NewMesh.MeshData.CastShadow)
				continue;
			Vector3 InvertedRotation = (i->GetTransform().Rotation + c->RelativeTransform.Rotation);
			InvertedRotation = Vector3(-InvertedRotation.Z, InvertedRotation.Y, -InvertedRotation.X);
			Transform ModelTransform = (Transform(Vector3::TranslateVector(c->RelativeTransform.Position, i->GetTransform()),
				Vector3() - InvertedRotation.DegreesToRadians(),
				c->RelativeTransform.Scale * 0.025f * i->GetTransform().Scale));

			glm::mat4 ModelMatrix = ModelTransform.ToMatrix();
			for (ModelGenerator::ModelData::Element& elem : NewMesh.MeshData.Elements)
			{
				for (Vertex& vert : elem.Vertices)
				{
					vert.Position = ModelMatrix * glm::vec4(vert.Position, 1);
				}
			}
			NewMesh.MeshTransform = ModelTransform;
			Bake::Meshes.push_back(NewMesh);
		}
	}
	Bake::Lights = Graphics::MainFramebuffer->Lights;

	new std::thread([]() {

		Application::Timer BakeTimer;
		BakeLogMessages.clear();
		BakeLog("Baking lightmap for scene " + FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene) + "...");
		for (auto& i : Bake::ThreadProgress)
		{
			i = 0;
		}
		Bake::Texture = new std::byte[LightmapResolution * LightmapResolution * LightmapResolution * NUM_CHANNELS]();

		std::vector<std::thread*> BakeThreads;

		Collision::Box sbox;

		for (auto& i : Bake::Meshes)
		{
			if (!i.MeshData.CastStaticShadow)
			{
				continue;
			}
			auto verts = i.MeshData.GetMergedVertices();
			for (auto& vert : verts)
			{
				if (vert.Position.x > sbox.maxX)
				{
					sbox.maxX = vert.Position.x;
				}
				if (vert.Position.y > sbox.maxY)
				{
					sbox.maxY = vert.Position.y;
				}
				if (vert.Position.z > sbox.maxZ)
				{
					sbox.maxZ = vert.Position.z;
				}
				if (vert.Position.x < sbox.minX)
				{
					sbox.minX = vert.Position.x;
				}
				if (vert.Position.y < sbox.minY)
				{
					sbox.minY = vert.Position.y;
				}
				if (vert.Position.z < sbox.minZ)
				{
					sbox.minZ = vert.Position.z;
				}
			}
		}

		Bake::BakeScale = (sbox.GetExtent() * 2) + sbox.GetCenter();
		Bake::BakeScale = std::max(Bake::BakeScale.X, std::max(Bake::BakeScale.Y, Bake::BakeScale.Z));
		BakeLog("Calculated scene bounding box: " + std::to_string(Bake::BakeScale.X));
		Bake::BakeScale = Bake::BakeScale * LightmapScaleMultiplier;
		BakeLog("Baking with scale: " + std::to_string(Bake::BakeScale.X));
		Bake::SunDirection = (glm::vec3)Vector3::GetForwardVector(Graphics::WorldSun.Rotation);
		size_t Thread3DArraySize = Bake::NUM_CHUNK_SPLITS;

		size_t ThreadID = 0;
		for (size_t x = 0; x < Thread3DArraySize; x++)
		{
			for (size_t y = 0; y < Thread3DArraySize; y++)
			{
				for (size_t z = 0; z < Thread3DArraySize; z++)
				{
					BakeThreads.push_back(new std::thread(Bake::BakeSection, x, y, z, ThreadID++));
				}
			}
		}

		BakeLog("Invoked " + std::to_string(BakeThreads.size()) + " threads");

		for (int i = (int)BakeThreads.size() - 1; i >= 0; i--)
		{
			BakeThreads[i]->join();
			BakeLog("Thread " + std::to_string(BakeThreads.size() - i) + "/" + std::to_string(BakeThreads.size()) + " is done.");
		}

		BakeLog("Finished baking lightmap.");
		BakeLog("Bake took " + std::to_string((int)BakeTimer.Get()) + " seconds.");

		for (int64_t x = 0; x < (int64_t)BakedLighting::LightmapResolution; x++)
		{
			for (int64_t y = 0; y < (int64_t)BakedLighting::LightmapResolution; y++)
			{
				for (int64_t z = 0; z < (int64_t)BakedLighting::LightmapResolution; z++)
				{
					uint16_t val = 0;
					for (int64_t bx = -1; bx <= 1; bx++)
					{
						for (int64_t bz = -1; bz <= 1; bz++)
						{
							val += (uint16_t)Sample3DArray(Bake::Texture, x + bx, y, z + bz);
						}
					}
					Bake::Texture[x * BakedLighting::LightmapResolution * BakedLighting::LightmapResolution
						+ y * BakedLighting::LightmapResolution
						+ z] = (std::byte)(val / 9);
				}
			}
		}
		
		// Simple RLE for lightmap compression.
		std::byte* TexPtr = Bake::Texture;

		std::string BakFile = Assets::GetAsset(FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene) + ".bkdat");
		if (!std::filesystem::exists(BakFile))
		{
			BakFile = FileUtil::GetFilePathWithoutExtension(Scene::CurrentScene) + ".bkdat";
		}


		std::ofstream OutFile = std::ofstream(BakFile);

		OutFile << (int)BKDAT_FILE_VERSION;
		struct RLEelem
		{
			std::byte Value = std::byte(1);
			size_t Length = 0;
		};

		RLEelem Current;
		std::vector<RLEelem> Elements;

		BakeLog("Compressing lightmap...");
		for (size_t i = 0; i < LightmapResolution * LightmapResolution * LightmapResolution * NUM_CHANNELS; i++)
		{
			std::byte CurrentVal = Bake::Texture[i];
			if (Current.Value != CurrentVal)
			{
				if (Current.Length != 0)
				{
					Elements.push_back(Current);
				}
				Current.Value = CurrentVal;
				Current.Length = 0;
			}
			Current.Length++;
		}

		size_t ElemsSize = Elements.size();
		OutFile << " " << ElemsSize << " " << LightmapResolution << std::endl;
		OutFile << Bake::BakeScale.ToString() << std::endl;

		size_t TotalLength = 0;

		BakeLog("Compressed lightmap with " + std::to_string(ElemsSize) + " RLE elements.");
		for (auto& i : Elements)
		{
			std::byte Val = i.Value;
			size_t Length = i.Length;
			TotalLength += Length;
			OutFile << Length << " ";
			OutFile << (int)Val << std::endl;
		}

		BakeLog("Encoded voxels: " + std::to_string(TotalLength));

		delete[] Bake::Texture;
		Bake::Meshes.clear();
		BakedLighting::FinishedBaking = true;
		});
}


float BakedLighting::GetBakeProgress()
{
	float Progress = 0;
	for (float i : Bake::ThreadProgress)
	{
		Progress += i;
	}
	return Progress / (Bake::NUM_CHUNK_SPLITS * Bake::NUM_CHUNK_SPLITS * Bake::NUM_CHUNK_SPLITS);
}


#endif

void BakedLighting::LoadBakeFile(std::string BakeFile)
{
	std::string File;
	if (!std::filesystem::exists(BakeFile))
	{
		File = Assets::GetAsset(BakeFile + ".bkdat");
	}
	else
	{
		File = BakeFile;
	}
	if (!std::filesystem::exists(File))
	{
		Log::Print("Could not find .bkdat file: " + BakeFile);
		BakedLighting::Init();
		return;
	}

	std::ifstream InFile = std::ifstream(File);

	int FileVer = 0;
	Log::Print("Loading lightmap: " + File);
	InFile >> FileVer;
	if (BKDAT_FILE_VERSION != FileVer)
	{
		Log::Print("File version mismatch of bkdat file. Supported version: " + std::to_string(BKDAT_FILE_VERSION) + ", Loaded version: " + std::to_string(FileVer), Log::LogColor::Red);
		BakedLighting::Init();
		return;
	}


	size_t FileLength = 0;
	InFile >> FileLength;
	InFile >> LightmapResolution;
	size_t Iterator = 0;

	float ScaleX, ScaleY, ScaleZ;

	InFile >> ScaleX;
	InFile >> ScaleY;
	InFile >> ScaleZ;

	LightmapScale = Vector3(ScaleX, ScaleY, ScaleZ);

	uint8_t* Texture = new uint8_t[LightmapResolution * LightmapResolution * LightmapResolution * NUM_CHANNELS * 2]();


	for (size_t i = 0; i < FileLength; i++)
	{
		int Value = 0;
		size_t Length = 0;
		InFile >> Length;
		InFile >> Value;
		for (size_t i = 0; i < Length; i++)
		{
			Texture[Iterator] = Value;
			Iterator++;
		}
	}

	LoadedLightmap = true;
	glDeleteTextures(1, &LightTexture);
	glGenTextures(1, &LightTexture);
	glBindTexture(GL_TEXTURE_3D, LightTexture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	float borderColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexImage3D(GL_TEXTURE_3D,
		0, 
		GL_R8,
		(GLsizei)LightmapResolution, 
		(GLsizei)LightmapResolution,
		(GLsizei)LightmapResolution,
		0, 
		GL_RED,
		GL_UNSIGNED_BYTE, 
		Texture);
	delete[] Texture;
}

Vector3 BakedLighting::GetLightMapScale()
{
	return LightmapScale;
}

std::vector<std::string> BakedLighting::BakeLogMessages;

void BakedLighting::BakeLog(std::string Msg)
{
	BakeLogMessages.push_back(Msg);
}

std::vector<std::string> BakedLighting::GetBakeLog()
{
	return BakeLogMessages;
}
