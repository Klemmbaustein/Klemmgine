using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public static class Scene
{
	private delegate void LoadSceneDelegate([MarshalAs(UnmanagedType.LPUTF8Str)] string SceneName);
	public static void LoadScene(string SceneName)
	{
		NativeFunction.CallNativeFunction("LoadScene", typeof(LoadSceneDelegate), new object[] { SceneName });
	}
}