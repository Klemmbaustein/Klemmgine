using Engine;
using Engine.Native;
using System;
using System.Runtime.InteropServices;

namespace Engine;

/**
 * @brief
 * PointLightComponent in C#.
 * 
 * Can be attached to any Engine.SceneObject.
 * 
 * C++ equivalent: PointLightComponent.
 * 
 * @ingroup CSharp-Components
 */
public class PointLightComponent : ObjectComponent
{
	private delegate IntPtr NewPointLightComponent(IntPtr Parent);
	private delegate void SetPointLightFloat(IntPtr Target, float Value);
	private delegate float GetPointLightFloat(IntPtr Target);
	private delegate void SetPointLightVector(IntPtr Target, Vector3 Value);
	private delegate Vector3 GetPointLightVector(IntPtr Target);

	public override void OnAttached()
	{
		NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewPointLightComponent", typeof(NewPointLightComponent), [Parent.NativePtr]);
	}

	/// The intensity of the light.
	public float Intensity
	{
		get
		{
			return (float)NativeFunction.CallNativeFunction("GetPointLightIntensity", typeof(GetPointLightFloat), [NativePtr]);
		}
		set
		{
			NativeFunction.CallNativeFunction("SetPointLightIntensity", typeof(SetPointLightFloat), [NativePtr, value]);
		}
	}

	/// The falloff of the light.
	public float Falloff
	{
		get
		{
			return (float)NativeFunction.CallNativeFunction("GetPointLightFalloff", typeof(GetPointLightFloat), [NativePtr]);
		}
		set
		{
			NativeFunction.CallNativeFunction("SetPointLightFalloff", typeof(SetPointLightFloat), [NativePtr, value]);
		}
	}

	/// The light color.
	public Vector3 Color
	{
		get
		{
			return (Vector3)NativeFunction.CallNativeFunction("GetPointLightColor", typeof(GetPointLightVector), [NativePtr]);
		}
		set
		{
			NativeFunction.CallNativeFunction("SetPointLightFalloff", typeof(SetPointLightVector), [NativePtr, value]);
		}
	}

	public override void Tick()
	{
	}
}