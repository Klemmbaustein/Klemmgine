#pragma once
#include <cstdint>
#include <string>
#include <atomic>
#include <Math/Vector.h>

class BakedLighting
{
	static unsigned int LightTexture;
	static std::vector<std::string> BakeLogMessages;
public:
	static void Init();

	static float GetLightIntensityAt(int64_t x, int64_t y, int64_t z, float ElemaSize);
	static void BindToTexture();
	static int64_t GetLightTextureSize();
	static Vector3 GetLightMapScale();
	static void BakeCurrentSceneToFile();
	static void LoadBakeFile(std::string BakeFile);

	static bool LoadedLightmap;

	static void Update();

	static float GetBakeProgress();

	static std::atomic<bool> FinishedBaking;
	static float LightmapScaleMultiplier;
	static uint64_t LightmapResolution;

	static std::vector<std::string> GetBakeLog();
	static void BakeLog(std::string Msg);
};