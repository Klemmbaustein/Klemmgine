#pragma once
#include <Math/Vector.h>

struct Shader;
class Camera;
struct VertexBuffer; struct IndexBuffer;
struct Uniform;
namespace Particles
{
	struct ParticleElement
	{
		float SpawnDelay = 0.1f;
		int NumLoops = -1;
		int RunLoops = -1;
		float Size = 1.f;
		float LifeTime = 3.f;
		Vector3 Direction = Vector3(0, 15, 0);
		Vector3 DirectionRandom = Vector3(5, 5, 5);
		Vector3 PositionRandom = Vector3(0);
		Vector3 Force = Vector3(0);
		float StartScale = 1;
		float EndScale = 1;
	};
	struct ParticleInstance
	{
		float InitialLifeTime;
		float LifeTime;
		Vector3 Velocity;
		Vector3 Position;
		Vector3 Force;
		float Scale;
		float StartScale;
		float EndScale;
		bool operator==(const ParticleInstance& b) const = default;
	};


	class ParticleEmitter
	{
	public:
		Vector3 Position;
		Vector3 Rotation;
		bool Active = true;
		static std::vector<ParticleElement> LoadParticleFile(std::string File, std::vector<std::string>& Materials);
		static void SaveToFile(std::vector<ParticleElement> Data, std::vector<std::string> Materials, std::string File);

		std::vector<float> SpawnDelays;
		std::vector<ParticleElement> ParticleElements;
		std::vector<std::vector<ParticleInstance>> ParticleInstances;

		void AddElement(ParticleElement NewElement);
		void RemoveElement(unsigned int Index);
		std::vector<VertexBuffer*> ParticleVertexBuffers;
		std::vector<IndexBuffer*> ParticleIndexBuffers;
		unsigned int MatBuffer = -1;
		std::vector<glm::mat4> ParticleMatrices;
		std::vector<Shader*> ParticleShaders;
		std::vector<std::vector<Uniform>> Uniforms;
		void UpdateParticlePositions(Camera* MainCamera);
		void AddParticleInstance(unsigned int Element);
		void ApplyUniforms(unsigned int Element);
		void ApplyUniform(int Index, unsigned int Element);
		ParticleEmitter();
		~ParticleEmitter();
		void SetMaterial(unsigned int Index, std::string Material, bool SupressMaterialError = false);
		void Reset();
		void Update(Camera* MainCamera);
		void Draw(Camera* WorldCamera, bool MainFrameBuffer, bool TransparencyPass);
		bool IsActive = true;
	};
}