using System;
using System.Runtime.InteropServices;
namespace Engine;

/**
 * @brief
 * MeshComponent in C#.
 * 
 * Can be attached to any Engine.WorldObject.
 * 
 * C++ equivalent: MeshComponent.
 * 
 * @ingroup CSharp-Components
 */
public class MeshComponent : ObjectComponent
{
	private delegate IntPtr NewModel([MarshalAs(UnmanagedType.LPUTF8Str)] string Filename, IntPtr Parent);


	public MeshComponent()
	{
	}

	/**
	* @brief
	* Loads a mesh file (.jsm) from the given file name. (without path or extension)
	* 
	* @param Filename
	* 
	*/
	public void Load(string File)
	{
		if (Parent == null)
		{
			return;
		}
		if (!NativePtr.Equals(new IntPtr()))
		{
			NativeFunction.CallNativeFunction("DestroyComponent", typeof(DestroyComponent), new object[] { NativePtr, Parent.NativePtr });
		}
		NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewMeshComponent", typeof(NewModel), new object[] { File, Parent.NativePtr });
	}

	public override void Tick()
	{
	}

	public override void Destroy()
	{
		NativeFunction.CallNativeFunction("DestroyComponent", typeof(DestroyComponent), new object[] { NativePtr, Parent.NativePtr });
		NativePtr = new IntPtr();
	}
}