using System;
using System.Runtime.InteropServices;

namespace Engine.UI;

/**
 * @brief
 * A class.
 * 
 * C++ equivalent: UICanvas
 * A class for managing multiple UI elements.
 * 
 * This class responds to button events with the OnButtonClicked() function.
 * 
 * @ingroup CSharp-UI
 */
public abstract class UICanvas
{
	public readonly IntPtr NativePtr;

	private delegate IntPtr NewUICanvas(IntPtr OnButtonClickedPtr, IntPtr UpdatePtr, IntPtr DestroyPtr);
	private delegate void OnButtonClickedDelegate(Int32 Index);
	private delegate void VoidDelegate();
	private delegate void DestroyDelegate(IntPtr Canvas);

	public UICanvas()
	{
		OnButtonClickedDelegate OnClicked = OnButtonClicked;
		VoidDelegate UpdateDelegate = Update;
		VoidDelegate DestroyDelegate = OnDestroyed;
		NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewUICanvas",
			typeof(NewUICanvas),
			[
				Marshal.GetFunctionPointerForDelegate(OnClicked),
				Marshal.GetFunctionPointerForDelegate(UpdateDelegate),
				Marshal.GetFunctionPointerForDelegate(DestroyDelegate),
			]);
	}

	/**
	 * @brief
	 * Called when a button belonging to this canvas gets clicked.
	 * 
	 * @param Index
	 * The index of the clicked button.
	 */
	protected abstract void OnButtonClicked(int Index);

	/**
	 * @brief
	 * Called each frame.
	 */
	protected abstract void Update();

	/**
	 * @brief
	 * Called when the UICanvas is destroyed.
	 * 
	 * This will be called if a new scene is loaded or the UICanvas has been destroyed with
	 * the Engine.UI.UICanvas.Destroy() method.
	 */
	protected abstract void OnDestroyed();

	/**
	 * @brief
	 * Deletes the UICanvas, calls OnDestroyed().
	 */
	public void Destroy()
	{
		NativeFunction.CallNativeFunction("DestroyUICanvas",
			typeof(DestroyDelegate),
			[
				NativePtr
			]);
	}

}
