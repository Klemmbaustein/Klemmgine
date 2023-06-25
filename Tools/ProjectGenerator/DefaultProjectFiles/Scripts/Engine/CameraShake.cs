using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

public static class CameraShake
{
	private delegate void CameraShakeDelegate(float i);

	public static void PlayCameraShake(float Intensity)
	{
		NativeFunction.CallNativeFunction("PlayDefaultCameraShake", typeof(CameraShakeDelegate), new object[] { Intensity });
	}
}