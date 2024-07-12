#include <Rendering/Texture/Material.h>
#include <fstream>
#include <Engine/File/Assets.h>
#include <Engine/Log.h>
#include <sstream>
#include <map>
#include <filesystem>
#include <Engine/Utility/FileUtility.h>
#include <Engine/EngineError.h>
#include <Engine/Application.h>
#include <Rendering/Mesh/Model.h>
#include <Rendering/Mesh/Mesh.h>
#include <Rendering/Graphics.h>
#include <Rendering/Framebuffer.h>
#include <Rendering/Mesh/InstancedModel.h>
#include <Rendering/Mesh/InstancedMesh.h>
#include <Engine/File/SaveData.h>
#include "Texture.h"

void Material::SetPredefinedMaterialValue(std::string Value, char* ptr, std::string Name)
{
	if (Value.empty())
	{
		Log::Print("Error while loading material file: The value of '" + Name + "' should be a number, but it is empty", Log::LogColor::Red);
		*ptr = false;
	}
	try
	{
		char val = std::stoi(Value);
		*ptr = val;
	}
	catch (const std::exception&)
	{
		Log::Print("Error while loading material file: The value of '" + Name + "' should be a number, but it is '" + Value + "'", Log::LogColor::Red);
		*ptr = false;
	}
}

Material Material::LoadMaterialFile(std::string Name)
{
	std::string File;
	std::string Ext;
	if (FileUtil::GetExtension(Name).empty())
	{
		Ext = ".jsmat";
	}

	if (!std::filesystem::exists(Name + Ext))
	{
		File = Assets::GetAsset(Name + Ext);
	}	
	if (Name.substr(0, 8) == "Content/" && !std::filesystem::exists(File))
	{
		File = Assets::GetAsset(Name.substr(8) + Ext);
	}
	if (Name.substr(0, 5) == "../..")
	{
		File = Application::GetEditorPath() + Name.substr(5) + Ext;
	}

	if (!std::filesystem::exists(File))
	{
		File = Name + Ext;
	}
	if (!std::filesystem::exists(File))
	{
		Log::Print("Could not load material: " + Name, Log::LogColor::Yellow);
		File = Application::GetEditorPath() + "/EditorContent/Materials/EngineDefaultPhong.jsmat";
#ifdef RELEASE
		ENGINE_ASSERT(std::filesystem::exists(File), "Could not load material: " + Name + ".\n\
This is a fatal error on release builds.\n\
Ensure that all of your models have a material assigned.");
#endif
	}

	SaveData MaterialData = SaveData(File, "", false, false);

	Material Out;
	Out.Name = File;
	for (auto& i : MaterialData.GetAllFields())
	{
		if (i.Name == "VertexShader")
		{
			Out.VertexShader = i.AsString();
			continue;
		}
		if (i.Name == "FragmentShader")
		{
			Out.FragmentShader = i.AsString();
			continue;
		}

		if (i.Name == "UseShadowCutout")
		{
			Out.UseShadowCutout = i.AsBool();
			continue;
		}
		if (i.Name == "IsTranslucent")
		{
			Out.IsTranslucent = i.AsBool();
			continue;
		}

		if (i.Type == NativeType::GL_Texture)
		{
			if (i.Data.empty())
			{
				Texture::TextureInfo Info;
				Info.File = i.At("File").AsString();
				Info.Filtering = (Texture::TextureFiltering)i.At("Filtering").AsInt();
				Info.Wrap = (Texture::TextureWrap)i.At("Wrap").AsInt();
				i.Data = Texture::CreateTextureInfoString(Info);
			}
		}

		Param p = Param(i.Name, i.Type, i.Data);
		Out.Uniforms.push_back(p);
	}

	return Out;
}

void Material::SaveMaterialFile(std::string Path, Material m)
{
	SaveData MaterialData = SaveData(Path, "", false);
	MaterialData.ClearFields();

	MaterialData.SetField(SaveData::Field(NativeType::String, "VertexShader", m.VertexShader));
	MaterialData.SetField(SaveData::Field(NativeType::String, "FragmentShader", m.FragmentShader));
	MaterialData.SetField(SaveData::Field(NativeType::Bool, "UseShadowCutout", std::to_string(m.UseShadowCutout)));
	MaterialData.SetField(SaveData::Field(NativeType::Bool, "IsTranslucent", std::to_string(m.IsTranslucent)));

	for (auto& i : m.Uniforms)
	{
		if (i.NativeType == NativeType::GL_Texture)
		{
			Texture::TextureInfo Info = Texture::ParseTextureInfoString(i.Value);
			SaveData::Field Field = SaveData::Field(NativeType::GL_Texture, i.UniformName, "");
			Field.Children =
			{
				SaveData::Field(NativeType::String, "File", Info.File),
				SaveData::Field(NativeType::Int, "Filtering", std::to_string(int(Info.Filtering))),
				SaveData::Field(NativeType::Int, "Wrap", std::to_string(int(Info.Wrap)))
			};
			MaterialData.SetField(Field);
			continue;
		}
		MaterialData.SetField(SaveData::Field(i.NativeType, i.UniformName, i.Value));
	}
}

#if !SERVER
void Material::ReloadMaterial(std::string MaterialPath)
{
	Material NewMaterial = LoadMaterialFile(MaterialPath);
	for (Drawable* m : Graphics::MainFramebuffer->Renderables)
	{
		Model* RenderableModel = dynamic_cast<Model*>(m);
		if (RenderableModel)
		{
			for (Mesh* i : RenderableModel->Meshes)
			{
				if (i->RenderContext.Mat.Name == MaterialPath)
				{
					i->RenderContext = ObjectRenderContext(NewMaterial);
				}
			}
		}
		InstancedModel* RenderableInstanced = dynamic_cast<InstancedModel*>(m);
		if (RenderableInstanced)
		{
			for (InstancedMesh* i : RenderableInstanced->Meshes)
			{
				if (i->RenderContext.Mat.Name == MaterialPath)
				{
					i->RenderContext = ObjectRenderContext(NewMaterial);
				}
			}
		}
	}
}
#endif