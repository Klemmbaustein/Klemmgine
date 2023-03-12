#include <Math/Vector.h>

namespace CameraShake
{
	extern Vector3 CameraShakeTranslation;

	void PlayDefaultCameraShake(float Intensity);
	void StopAllCameraShake();
	float GetCurrentCameraShakeIntensity();
	void Tick();
}