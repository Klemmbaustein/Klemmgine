#if !SERVER
#pragma once
#include <cstdint>
#include <string>
#include <atomic>
#include <Math/Vector.h>
#include "RenderSubsystem.h"

/**
* @brief
* Baked lighting subsystem.
* 
* @ingroup Internal
* @ingroup Subsystem
*/
class BakedLighting : public RenderSubsystem
{
	static unsigned int LightTexture;
	static std::vector<std::string> BakeLogMessages;
	static BakedLighting* BakeSystem;
public:
	BakedLighting();

	static void LoadEmpty();

	static void BindToTexture();
	static int64_t GetLightTextureSize();
	static Vector3 GetLightMapScale();
	/// Bakes the currently opened scene to a file.
	static void BakeCurrentSceneToFile();

	/// Loads a lightmap file.
	static void LoadBakeFile(std::string BakeFile);

	/// True if a lightmap is loaded, false if not.
	static bool LoadedLightmap;

	void Update() override;

	/// Gets the progress of the lighting bake process.
	static float GetBakeProgress();

	/// True if baking lighting is done.
	static std::atomic<bool> FinishedBaking;
	static float LightmapScaleMultiplier;
	static uint64_t LightmapResolution;

	static std::vector<std::string> GetBakeLog();
private:
	static float GetLightIntensityAt(int64_t x, int64_t y, int64_t z, float ElemaSize);
	static void BakeLog(std::string Msg);
	static void LoadBakeTexture(uint8_t* Texture);
};
#endif