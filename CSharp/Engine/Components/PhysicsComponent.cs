namespace Engine;
using Engine.Native;
using System;
using System.Runtime.InteropServices;

/**
* @brief
* A component that simulates physics.
* 
* @ingroup CSharp-Components
*/
public class PhysicsComponent : ObjectComponent
{
	/// The type a physics body can have.
	enum BodyType
	{
		Box,
		Sphere,
		Capsule,
		Mesh,
	};

	public override void Tick()
	{
	}


	private delegate IntPtr NewPhysicsComponentDelegate(
		IntPtr Parent,
		Transform t,
		BodyType Type,
		Collision.PhysicsMotionType Movability,
		Collision.Layer Layers);

	private delegate Transform GetTransformDelegate(IntPtr NativePtr);
	private delegate void SetVector3Delegate(IntPtr NativePtr, Vector3 Vec);
	private delegate Vector3 GetVector3Delegate(IntPtr NativePtr);
	private delegate void SetBoolDelegate(IntPtr NativePtr, [MarshalAs(UnmanagedType.U1)] bool NewActive);
	private delegate Collision.HitResponse ShapeCastDelegate(
		IntPtr NativePtr,
		Transform Start,
		Vector3 End,
		Collision.Layer Layers,
		[MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] IntPtr[] ComponentsToIgnore,
		int Length);

	private delegate Collision.HitResponse CollisionCheckDelegate(
	IntPtr NativePtr,
	Transform Where,
	Collision.Layer Layers,
	[MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] IntPtr[] ComponentsToIgnore,
	int Length);


	private void CreateColliderOfType(BodyType Type,
		Transform RelativeTransform,
		Collision.PhysicsMotionType Movability,
		Collision.Layer CollisionLayers)
	{
		NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewPhysicsComponent", typeof(NewPhysicsComponentDelegate),
			[
			Parent.NativePtr,
			RelativeTransform,
			Type,
			Movability,
			CollisionLayers
			]);
	}

	/// Sets the position of the physics body, in world space.
	public void SetPosition(Vector3 NewPosition)
	{
		NativeFunction.CallNativeFunction("PhysicsComponentSetPosition", typeof(SetVector3Delegate), [NativePtr, NewPosition]);
	}

	/// Sets the rotation of the physics body, in world space.
	public void SetRotation(Vector3 NewRotation)
	{
		NativeFunction.CallNativeFunction("PhysicsComponentSetRotation", typeof(SetVector3Delegate), [NativePtr, NewRotation]);
	}

	/// Sets the scale of the physics body, in world space.
	public void SetScale(Vector3 NewScale)
	{
		NativeFunction.CallNativeFunction("PhysicsComponentSetScale", typeof(SetVector3Delegate), [NativePtr, NewScale]);
	}

	/// Sets the velocity of the physics body.
	public void SetVelocity(Vector3 NewVelocity)
	{
		NativeFunction.CallNativeFunction("PhysicsComponentSetVelocity", typeof(SetVector3Delegate), [NativePtr, NewVelocity]);
	}

	/// Sets the angular velocity of the physics body.
	public void SetAngularVelocity(Vector3 NewVelocity)
	{
		NativeFunction.CallNativeFunction("PhysicsComponentSetAngularVelocity", typeof(SetVector3Delegate), [NativePtr, NewVelocity]);
	}

	/// Gets the velocity of the physics body.
	public Vector3 GetVelocity()
	{
		return (Vector3)NativeFunction.CallNativeFunction("PhysicsComponentGetVelocity", typeof(GetVector3Delegate), [NativePtr]);
	}

	/// Gets the angular velocity of the physics body.
	public Vector3 GetAngularVelocity()
	{
		return (Vector3)NativeFunction.CallNativeFunction("PhysicsComponentGetAngularVelocity", typeof(GetVector3Delegate), [NativePtr]);
	}

	/// Gets the transform of the simulated physics body, in world space.
	public Transform GetBodyWorldTransform()
	{
		return (Transform)NativeFunction.CallNativeFunction("PhysicsComponentGetTransform", typeof(GetTransformDelegate), [NativePtr]);
	}

	/**
	* @brief
	* Creates a @ref Physics::BoxBody with the given parameters.
	*
	* @param RelativeTransform
	* The Transform of the PhysicsBody, relative to this component's parent.
	* @param BoxMovability
	* The movability of the PhysicsBody.
	* @param CollisionLayers
	* The layers of the PhysicsBody.
	*/
	public void CreateBox(
		Transform RelativeTransform,
		Collision.PhysicsMotionType BoxMovability,
		Collision.Layer CollisionLayers)
	{
		CreateColliderOfType(BodyType.Box, RelativeTransform, BoxMovability, CollisionLayers);
	}

	/**
	* @brief
	* Creates a @ref Physics::SphereBody with the given parameters.
	*
	* @param RelativeTransform
	* The Transform of the PhysicsBody, relative to this component's parent.
	* @param SphereMovability
	* The movability of the PhysicsBody.
	* @param CollisionLayers
	* The layers of the PhysicsBody.
	*/
	public void CreateSphere(
		Transform RelativeTransform,
		Collision.PhysicsMotionType SphereMovability,
		Collision.Layer CollisionLayers)
	{
		CreateColliderOfType(BodyType.Sphere, RelativeTransform, SphereMovability, CollisionLayers);
	}

	/**
	* @brief
	* Creates a @ref Physics::CapsuleBody with the given parameters.
	*
	* @param RelativeTransform
	* The Transform of the PhysicsBody, relative to this component's parent.
	* @param CapsuleMovability
	* The movability of the PhysicsBody.
	* @param CollisionLayers
	* The layers of the PhysicsBody.
	*/
	public void CreateCapsule(
		Transform RelativeTransform,
		Collision.PhysicsMotionType CapsuleMovability,
		Collision.Layer CollisionLayers)
	{
		CreateColliderOfType(BodyType.Capsule, RelativeTransform, CapsuleMovability, CollisionLayers);
	}

	/**
	* @brief
	* Sets the activeness of the body
	* 
	* Active means the collider can interact with the physics system.
	* Inactive means it can't.
	*/
	public void SetActive(bool NewActive)
	{
		NativeFunction.CallNativeFunction("PhysicsComponentSetActive", typeof(SetBoolDelegate), [NativePtr, NewActive]);
	}

	/**
	* @brief
	* Casts this physics component shape from Start to End.
	* 
	* @param Start
	* The start Engine.Transform of the cast.
	* @param End
	* The end position of the cast.
	* @param Layers
	* The layers to check.
	* @param ObjectsToIgnore
	* These objects shouldn't be considered for the query.
	*/
	public Collision.HitResponse ShapeCast(Transform Start, Vector3 End, Collision.Layer Layers, WorldObject[] ObjectsToIgnore = null)
	{
		IntPtr[] ComponentPtrs;
		if (ObjectsToIgnore == null)
		{
			ComponentPtrs = [];
		}
		else
		{
			ComponentPtrs = new IntPtr[ObjectsToIgnore.Length];

			for (int i = 0; i < ObjectsToIgnore.Length; i++)
			{
				ComponentPtrs[i] = ObjectsToIgnore[i].NativePtr;
			}
		}

		return (Collision.HitResponse)NativeFunction.CallNativeFunction(
			"PhysicsComponentShapeCast",
			typeof(ShapeCastDelegate),
			[NativePtr, Start, End, Layers, ComponentPtrs, ComponentPtrs.Length]
			);
	}

	/**
	* @brief
	* Queries collision with this physics component's shape.
	* 
	* @param Where
	* The Engine.Transform of the query.
	* @param Layers
	* The layers to check.
	* @param ObjectsToIgnore
	* These objects shouldn't be considered for the query.
	*/
	public Collision.HitResponse PhysicsComponentCollisionCheck(Transform Where, Collision.Layer Layers, WorldObject[] ObjectsToIgnore = null)
	{
		IntPtr[] ComponentPtrs;
		if (ObjectsToIgnore == null)
		{
			ComponentPtrs = [];
		}
		else
		{
			ComponentPtrs = new IntPtr[ObjectsToIgnore.Length];

			for (int i = 0; i < ObjectsToIgnore.Length; i++)
			{
				ComponentPtrs[i] = ObjectsToIgnore[i].NativePtr;
			}
		}

		return (Collision.HitResponse)NativeFunction.CallNativeFunction(
			"PhysicsComponentCollisionCheck",
			typeof(CollisionCheckDelegate),
			[NativePtr, Where, Layers, ComponentPtrs, ComponentPtrs.Length]
			);
	}
}