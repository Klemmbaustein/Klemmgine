#include "Renderable.h"
#include <GL/glew.h>
#include <Rendering/Utility/CSM.h>
#include <World/Graphics.h>
#include <World/Stats.h>
#include <Rendering/Utility/Framebuffer.h>


void Renderable::ApplyDefaultUniformsToShader(Shader* ShaderToApply)
{
	ShaderToApply->Bind();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, CSM::ShadowMaps);
	glUniform1i(glGetUniformLocation(ShaderToApply->GetShaderID(), "farPlane"), CSM::cameraFarPlane);
	glUniform1i(glGetUniformLocation(ShaderToApply->GetShaderID(), "cascadeCount"), CSM::shadowCascadeLevels.size());
	glUniform1i(glGetUniformLocation(ShaderToApply->GetShaderID(), "u_textureres"), Graphics::ShadowResolution);
	glUniform1i(glGetUniformLocation(ShaderToApply->GetShaderID(), "shadowMap"), 1);
	glUniform1i(glGetUniformLocation(ShaderToApply->GetShaderID(), "Skybox"), 2);
	glUniform1f(glGetUniformLocation(ShaderToApply->GetShaderID(), "u_biasmodifier"), (Vector3::Dot(
		Vector3::GetForwardVector(Graphics::MainCamera->Rotation),
		Graphics::WorldSun.Direction.Normalize())));
	Vector3 CameraForward = Vector3::GetForwardVector(Graphics::MainCamera->Rotation);
	glUniform3fv(glGetUniformLocation(ShaderToApply->GetShaderID(), "u_cameraforward"), 1, &CameraForward.X);
	glUniform3fv(glGetUniformLocation(ShaderToApply->GetShaderID(), "u_cameraposition"), 1, &Graphics::MainFramebuffer->FramebufferCamera->Position.x);
	glUniform1i(glGetUniformLocation(ShaderToApply->GetShaderID(), "u_shadowQuality"), Graphics::PCFQuality);
	glUniform1i(glGetUniformLocation(ShaderToApply->GetShaderID(), "u_shadows"), Graphics::RenderShadows);
	glUniform1f(glGetUniformLocation(ShaderToApply->GetShaderID(), "FogFalloff"), Graphics::WorldFog.Falloff);
	glUniform1f(glGetUniformLocation(ShaderToApply->GetShaderID(), "FogDistance"), Graphics::WorldFog.Distance);
	glUniform1f(glGetUniformLocation(ShaderToApply->GetShaderID(), "FogMaxDensity"), Graphics::WorldFog.MaxDensity);
	glUniform3fv(glGetUniformLocation(ShaderToApply->GetShaderID(), "FogColor"), 1, &Graphics::WorldFog.FogColor.X);

	glUniform3fv(glGetUniformLocation(ShaderToApply->GetShaderID(), "u_directionallight.Direction"), 1, &Graphics::WorldSun.Direction.X);
	glUniform1f(glGetUniformLocation(ShaderToApply->GetShaderID(), "u_directionallight.Intensity"), Graphics::WorldSun.Intensity);
	glUniform1f(glGetUniformLocation(ShaderToApply->GetShaderID(), "u_directionallight.AmbientIntensity"), Graphics::WorldSun.AmbientIntensity);
	glUniform3fv(glGetUniformLocation(ShaderToApply->GetShaderID(), "u_directionallight.SunColor"), 1, &Graphics::WorldSun.SunColor.X);
	glUniform3fv(glGetUniformLocation(ShaderToApply->GetShaderID(), "u_directionallight.AmbientColor"), 1, &Graphics::WorldSun.AmbientColor.X);

	glUniform1f(glGetUniformLocation(ShaderToApply->GetShaderID(), "u_time"), Stats::Time);

	for (size_t i = 0; i < CSM::shadowCascadeLevels.size(); ++i)
	{
		glUniform1fv(glGetUniformLocation(ShaderToApply->GetShaderID(),
			((std::string("cascadePlaneDistances[") + std::to_string(i)) + "]").c_str()), 1, &CSM::shadowCascadeLevels[i]);
	}
}
