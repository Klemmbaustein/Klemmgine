#version 430

struct DirectionalLight
{
	vec3 Direction;
	vec3 SunColor;
	vec3 AmbientColor;
	float Intensity;
	float AmbientIntensity;
};

struct PointLight
{
	vec3 Position;
	vec3 Color;
	float Intensity;
	float Falloff;
	bool Active;
};

uniform mat4 u_modelviewpro;
uniform mat4 u_modelview;
uniform mat4 u_model;

layout(location = 0) out vec4 f_color;
layout(location = 1) out vec4 f_normal;
layout(location = 2) out vec4 f_position;

in vec2 v_texcoord;
in vec3 v_position;
in vec3 v_modelnormal;
in vec3 v_screenposition;
in vec3 v_normal;
in vec3 v_screennormal;
in vec3 v_normmodelnormal;

uniform sampler2DArray shadowMap;
uniform mat4 u_view;
uniform int cascadeCount = 4;
uniform float cascadePlaneDistances[8];
uniform float u_biasmodifier;
uniform DirectionalLight u_directionallight;
uniform int u_shadowQuality;
uniform int u_textureres;
uniform vec3 u_cameraposition;
uniform int u_shadows;
uniform PointLight u_lights[8];
uniform bool u_ssao_reverse = true;
uniform samplerCube Skybox;

layout (std140, binding = 0) uniform LightSpaceMatrices
{
	mat4 lightSpaceMatrices[8];
};


uniform float FogFalloff;
uniform float FogDistance;
uniform float FogMaxDensity;
uniform vec3 FogColor;

vec4 ApplyFogColor(vec4 InColor)
{
	float Depth = length(v_screenposition);
	float Intensity = pow(min(max((Depth - FogDistance), 0) / FogFalloff, FogMaxDensity), 1);
	Intensity -= v_position.y / 500;
	return vec4(mix(InColor, vec4(FogColor * 0.7, 1), clamp((Intensity), 0, 1)).xyz, 1);
}

float SampleFromShadowMap(vec2 Texcoords, float bias, vec2 texelSize, int layer, vec2 distances, float currentDepth)
{
	int i = 0;
	float points[4];

	for (int x = 0; x <= 1; ++x)
	{
		for (int y = 0; y <= 1; ++y)
		{
			float pcfDepth;
			pcfDepth = texture(shadowMap, vec3(Texcoords + vec2(x, y) * texelSize.rg, layer)).r;
			points[i] = currentDepth > pcfDepth + bias ? 1.0 : 0.0;
			i++;
		}
	}
	return mix(mix(points[0], points[2], distances.x), mix(points[1], points[3], distances.x), distances.y);
}
float SampleFromShadowMapCheap(vec2 Texcoords, float bias, int layer, float currentDepth)
{
	float pcfDepth;
	pcfDepth = texture(shadowMap, vec3(Texcoords, layer)).r;
	return currentDepth > pcfDepth + bias ? 1.0 : 0.0;
}

float ShadowCalculation(vec3 fragPosWorldSpace, vec3 v_modelnormal)
{
	// select cascade layer
	vec4 fragPosViewSpace = u_view * vec4(fragPosWorldSpace, 1.0);
	float depthValue = abs(fragPosViewSpace.z);

	int layer = -1;
	for (int i = 0; i < cascadeCount; ++i)
	{
		if (depthValue < cascadePlaneDistances[i])
		{
			layer = i;
			break;
		}
	}
	if (layer == -1)
	{
		layer = cascadeCount;
	}
	if (layer == cascadeCount)
	{
		return 0.f;
	}
	vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0);

	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;

	// get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;

	// keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
	if (currentDepth > 1.0)
	{
		return 0.f;
	}
	vec2 texelSize = 1.f / textureSize(shadowMap, 0).xy;

	float bias = 0.0035f;


	bias += (1 - (abs(dot(normalize(v_modelnormal), normalize(u_directionallight.Direction))))) / 75;
	bias *= max((abs(u_biasmodifier * 2)), 0.5f) / 15.f;
	if (u_biasmodifier < -0.95)
		bias *= 2;
	bias *= 0.05;
	bias *= max(4096 / textureSize(shadowMap, 0).x*1.5f, 1);
	// PCF
	float shadow = 0.f;
	vec2 distances = vec2(mod(projCoords.x, texelSize.x), mod(projCoords.y, texelSize.y)) * vec2(u_textureres);
	int i = 0;
	for (int x = 0; x <= 1; x += 1)
	{
		for (int y = 0; y <= 1; y += 1)
		{
			shadow += SampleFromShadowMap(projCoords.xy + vec2(x, y) * (u_shadowQuality - 1) * texelSize, bias, texelSize, layer, distances, currentDepth);
			i++;
		}
	}
	shadow /= i;
	shadow -= 0.1f;
	shadow = max(shadow, 0);
	shadow *= 1.3f;
	shadow = min(shadow, 1.f);
	return shadow;
}

vec3 GetLightingNormal(vec3 color, float specularstrength, float specularsize, vec3 normal)
{
	vec3 view = normalize(v_position.xyz - u_cameraposition.xyz);
	vec3 reflection = reflect(u_directionallight.Direction, normal);
	float shadow = 1;
	if (u_shadows == 1)
	{
		shadow = 1 - ShadowCalculation(v_position, normal); //ShadowCalculation is expensive because of PCF
	}
	else
	{
		shadow = 1;
	}
	float specular = 1 * pow(max(dot(reflection, view), 0.000001), specularsize * 35) * specularstrength;
	vec3 light = (u_directionallight.Direction);

	//Cel shading (wow, very cool)
	//int Intensity = int(ceil(max(dot(normal, light), 0.f)));

	//Phong shading (wow, also very cool)
	float Intensity = max(dot(normal, light), 0.f);

	vec3 DirectionalLightColor = Intensity * (color * u_directionallight.Intensity) * (1 - u_directionallight.AmbientIntensity) * u_directionallight.SunColor;
	vec3 ambient = max(vec3(vec4(color, 1.f)) * vec3(u_directionallight.AmbientIntensity),  0.f) * u_directionallight.AmbientColor;

	if (specularstrength > 0)
	{
		vec3 I = normalize(v_position - u_cameraposition);
		vec3 R = reflect(I, normal);
		ambient *= (0.8 + texture(Skybox, -R).xyz * (specularstrength / 4));
	}

	vec3 lightingColor = vec3(0);

	for (int i = 0; i < 8; i++)
	{
		if (u_lights[i].Active)
		{
			vec3 pointLightDir = (u_lights[i].Position - v_position);
			float LightingIntensity = max(dot(normal, normalize(pointLightDir)), 0);
			vec3 newLightColor = vec3((u_lights[i].Falloff * 100) - (length(pointLightDir) * length(pointLightDir))) / (u_lights[i].Falloff * 20);
			newLightColor = pow(newLightColor, vec3(4));
			newLightColor *= u_lights[i].Color * u_lights[i].Intensity * LightingIntensity / 1000.f;
			newLightColor = max(newLightColor, 0);
			lightingColor += newLightColor;
		}
		else break;
	}
	lightingColor *= ambient / u_directionallight.AmbientIntensity;

	return ambient + (DirectionalLightColor + specular) * shadow + lightingColor;
}

vec3 GetLighting(vec3 color, float specularstrength, float specularsize)
{
	return GetLightingNormal(color, specularstrength, specularsize, normalize(v_modelnormal));
}


#define RETURN_COLOR(color) f_color = ApplyFogColor(vec4(color, 1));\
		f_normal = vec4(normalize(v_screennormal) * (!gl_FrontFacing && u_ssao_reverse ? -1 : 1), 1);\
		f_position = vec4(v_screenposition * 8, 1);

#define RETURN_COLOR_OPACITY(color, opacity) f_color = vec4(ApplyFogColor(vec4(color, 1)).xyz, opacity);\
		f_normal = vec4(normalize(v_screennormal) * (!gl_FrontFacing && u_ssao_reverse ? -1 : 1), 1);\
		f_position = vec4(v_screenposition * 8, 1);

#define RETURN_COLOR_NO_FOG(color) f_color = vec4(color, 1);\
		f_normal = vec4(normalize(v_screennormal) * (!gl_FrontFacing && u_ssao_reverse ? -1 : 1), 1);\
		f_position = vec4(v_screenposition * 8, 1);