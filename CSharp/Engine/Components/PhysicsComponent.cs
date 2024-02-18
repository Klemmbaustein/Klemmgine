namespace Engine;
using Engine.Native;
using System;

public class PhysicsComponent : ObjectComponent
{
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

	private void CreateColliderOfType(BodyType Type,
		Transform RelativeTransform,
		Collision.PhysicsMotionType Movability,
		Collision.Layer CollisionLayers)
	{
		Log.Print(Parent.GetName());
		NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewPhysicsComponent", typeof(NewPhysicsComponentDelegate),
			[
			Parent.NativePtr,
			RelativeTransform,
			Type,
			Movability,
			CollisionLayers
			]);
	}

	public void SetPosition(Vector3 NewPosition)
	{
		NativeFunction.CallNativeFunction("PhysicsComponentSetPosition", typeof(SetVector3Delegate), [NativePtr, NewPosition]);
	}

	public void SetRotation(Vector3 NewRotation)
	{
		NativeFunction.CallNativeFunction("PhysicsComponentSetRotation", typeof(SetVector3Delegate), [NativePtr, NewRotation]);
	}

	public void SetScale(Vector3 NewScale)
	{
		NativeFunction.CallNativeFunction("PhysicsComponentSetScale", typeof(SetVector3Delegate), [NativePtr, NewScale]);
	}

	public void SetVelocity(Vector3 NewVelocity)
	{
		NativeFunction.CallNativeFunction("PhysicsComponentSetVelocity", typeof(SetVector3Delegate), [NativePtr, NewVelocity]);
	}

	public void SetAngularVelocity(Vector3 NewVelocity)
	{
		NativeFunction.CallNativeFunction("PhysicsComponentSetAngularVelocity", typeof(SetVector3Delegate), [NativePtr, NewVelocity]);
	}

	public Vector3 GetVelocity()
	{
		return (Vector3)NativeFunction.CallNativeFunction("PhysicsComponentGetVelocity", typeof(GetVector3Delegate), [NativePtr]);
	}

	public Vector3 GetAngularVelocity()
	{
		return (Vector3)NativeFunction.CallNativeFunction("PhysicsComponentGetAngularVelocity", typeof(GetVector3Delegate), [NativePtr]);
	}

	public Transform GetBodyWorldTransform()
	{
		return (Transform)NativeFunction.CallNativeFunction("PhysicsComponentGetTransform", typeof(GetTransformDelegate), [NativePtr]);
	}


	public void CreateBox(
		Transform RelativeTransform,
		Collision.PhysicsMotionType BoxMovability,
		Collision.Layer CollisionLayers)
	{
		CreateColliderOfType(BodyType.Box, RelativeTransform, BoxMovability, CollisionLayers);
	}

	public void CreateSphere(
		Transform RelativeTransform,
		Collision.PhysicsMotionType SphereMovability,
		Collision.Layer CollisionLayers)
	{
		CreateColliderOfType(BodyType.Sphere, RelativeTransform, SphereMovability, CollisionLayers);
	}

	public void CreateCapsule(
		Transform RelativeTransform,
		Collision.PhysicsMotionType CapsuleMovability,
		Collision.Layer CollisionLayers)
	{
		CreateColliderOfType(BodyType.Capsule, RelativeTransform, CapsuleMovability, CollisionLayers);
	}

}