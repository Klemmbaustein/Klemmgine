using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public static class Sound
{
	private delegate IntPtr LoadSoundDelegate([MarshalAs(UnmanagedType.LPUTF8Str)] string SoundName);
	private delegate void UnloadSoundDelegate([MarshalAs(UnmanagedType.LPUTF8Str)] IntPtr SoundBuffer);
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
