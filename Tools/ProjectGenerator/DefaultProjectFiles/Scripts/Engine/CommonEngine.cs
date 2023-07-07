using System.Runtime.InteropServices;
using System;

// Common engine functions.
// TODO: think about putting this into a single namespace. (Like 'Engine')

public static class CameraShake
{
	private delegate void CameraShakeDelegate(float i);

	public static void PlayCameraShake(float Intensity)
	{
		NativeFunction.CallNativeFunction("PlayDefaultCameraShake", typeof(CameraShakeDelegate), new object[] { Intensity });
	}
}

public static class Console
{
	private delegate bool CommandDelegate([MarshalAs(UnmanagedType.LPUTF8Str)] string Command);
	public static bool ExecuteConsoleCommand([MarshalAs(UnmanagedType.LPUTF8Str)] string Command)
	{
		return (bool)NativeFunction.CallNativeFunction("CallConsoleCommand", typeof(CommandDelegate), new object[] { Command });
	}
}

public static class Collision
{
	private delegate HitResponse LineTraceDelegate(Vector3 Start, Vector3 End, IntPtr This);

	[StructLayout(LayoutKind.Sequential)]
	public struct HitResponse
	{
		public bool Hit;
		public Vector3 ImpactPoint;
		public float t;
		public IntPtr HitObject;
		public IntPtr HitComponent;
		public Vector3 Normal;
	}

	public static HitResponse LineTrace(Vector3 Start, Vector3 End, WorldObject This)
	{
		return (HitResponse)NativeFunction.CallNativeFunction("NativeRaycast", typeof(LineTraceDelegate), new object[] { Start, End, This.NativeObject });
	}
}

public static class Stats
{
	public static float DeltaTime = 0.0f;
	public static bool InEditor = false;
}


public static class Scene
{
	private delegate void LoadSceneDelegate([MarshalAs(UnmanagedType.LPUTF8Str)] string SceneName);
	public static void LoadScene(string SceneName)
	{
		NativeFunction.CallNativeFunction("LoadScene", typeof(LoadSceneDelegate), new object[] { SceneName });
	}
}

public static class Sound
{
	private delegate IntPtr LoadSoundDelegate([MarshalAs(UnmanagedType.LPUTF8Str)] string SoundName);
	private delegate void UnloadSoundDelegate(IntPtr SoundBuffer);
	private delegate void PlaySound(IntPtr s, float Pitch, float Volume, bool Looping);

	public class SoundBuffer
	{
		IntPtr BufferPtr = new();
		public SoundBuffer(IntPtr Buffer)
		{
			BufferPtr = Buffer;
		}

		public void Play(float Pitch = 1, float Volume = 1, bool Looping = false)
		{
			NativeFunction.CallNativeFunction("PlaySound", typeof(PlaySound), new object[] { BufferPtr, Pitch, Volume, Looping });
		}
		~SoundBuffer()
		{
			NativeFunction.CallNativeFunction("UnloadSound", typeof(UnloadSoundDelegate), new object[] { BufferPtr });
		}
	}

	public static SoundBuffer LoadSound(string File)
	{
		IntPtr BufferPtr = (IntPtr)NativeFunction.CallNativeFunction("LoadSound", typeof(LoadSoundDelegate), new object[] { File });
		return new SoundBuffer(BufferPtr);
	}
}
