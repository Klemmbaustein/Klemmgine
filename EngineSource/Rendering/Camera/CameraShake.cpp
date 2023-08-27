#include "CameraShake.h"
#include <Engine/Stats.h>

namespace CameraShake
{
	Vector3 CameraShakeTranslation;
}

float ShakeIntensity;

void CameraShake::PlayDefaultCameraShake(float Intensity)
{
	ShakeIntensity += Intensity;
}

void CameraShake::StopAllCameraShake()
{
	ShakeIntensity = 0;
}

void CameraShake::Tick()
{
	float ModifiedShakeIntensity = powf(ShakeIntensity, 2);
	CameraShakeTranslation = Vector3(sinf(Stats::Time * 30) * ModifiedShakeIntensity, sinf(Stats::Time * 25) * ModifiedShakeIntensity, 0);
	ShakeIntensity = std::max(ShakeIntensity - Performance::DeltaTime * 5, 0.0f);
}

float CameraShake::GetCurrentCameraShakeIntensity()
{
	return ShakeIntensity;
}
