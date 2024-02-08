#pragma once
#include <Rendering/Mesh/ModelGenerator.h>
#include <Math/Vector.h>

class Component;

namespace Physics
{
	void Init();
	void Update();

	/**
	* @brief
	* Physics layer enum. A physics object will only collide with objects that have the same layer.
	*/
	enum class Layer : uint16_t
	{
		None    = 0b00000000,
		Layer0  = 0b00000001,
		Layer1  = 0b00000010,
		Layer2  = 0b00000100,
		Layer3  = 0b00001000,
		Layer4  = 0b00010000,
		Trigger = 0b00100000,
		Static  = 0b01000000,
		Dynamic = 0b10000000
	};

	enum class MotionType
	{
		Static,
		Kinematic,
		Dynamic,
	};

	inline Layer operator|(Layer a, Layer b)
	{
		return static_cast<Layer>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
	}

	inline Layer operator&(Layer a, Layer b)
	{
		return static_cast<Layer>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
	}

	struct HitResult
	{
		bool Hit = false;
		float Depth = 0.0f;
		Component* HitComponent = nullptr;
		Vector3 ImpactPoint;
		Vector3 Normal;
	};

	struct PhysicsBody
	{
		enum class BodyType
		{
			Box,
			Sphere,
			Capsule,
			Mesh,
		};

		BodyType Type = BodyType::Box;


		MotionType ColliderMovability = MotionType::Static;
		Layer CollisionLayers = Layer::None;
		Component* Parent = nullptr;
		PhysicsBody(BodyType Type, Transform BodyTransform, MotionType ColliderMovability, Layer CollisionLayers, Component* Parent);

		const Transform& GetTransform() const;

		Vector3 GetPosition();
		Vector3 GetRotation();

		void SetPosition(Vector3 NewPosition);
		void SetRotation(Vector3 NewRotation);
		void Scale(Vector3 ScaleMultiplier);

		std::vector<HitResult> CollisionTest();

		void* PhysicsSystemBody = nullptr;
		void* ShapeInfo = nullptr;
	protected:
		Transform BodyTransform;
	private:
		virtual void SetTransform(Transform T) = 0;
	};

	struct SphereBody : public PhysicsBody
	{
		SphereBody(Vector3 Position, Vector3 Rotation, float Scale, MotionType ColliderMovability, Layer CollisionLayers, Component* Parent);
		void SetSphereTransform(Vector3 Position, Vector3 Rotation, float Scale);
	private:
		virtual void SetTransform(Transform T) override;
	};

	struct BoxBody : public PhysicsBody
	{
		BoxBody(Vector3 Position, Vector3 Rotation, Vector3 Extents, MotionType ColliderMovability, Layer CollisionLayers, Component* Parent);

		Vector3 Extents;
		virtual void SetTransform(Transform T) override;
	};

	struct MeshBody : public PhysicsBody
	{
		MeshBody(const ModelGenerator::ModelData& Mesh, Transform MeshTransform, MotionType ColliderMovability, Layer CollisionLayers, Component* Parent);
		virtual void SetTransform(Transform T) override;

		ModelGenerator::ModelData MeshData;
	};

	void AddBody(PhysicsBody* Body);
	void RemoveBody(PhysicsBody* Body);

	HitResult RayCast(Vector3 Start, Vector3 End);
}