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

	Material OutMaterial;
	OutMaterial.Name = File;
	std::ifstream In = std::ifstream(File);
	
	char CurrentBuff[100];

	//iterate through all lines which (hopefully) contain save values
	while (!In.eof())
	{

		NativeType::NativeType CurrentType = NativeType::Null;
		std::string CurrentName = "";
		std::string Value = "";

		std::string CurrentLine;
		In.getline(CurrentBuff, 100);
		CurrentLine = CurrentBuff;

		if (CurrentLine.substr(0, 2) == "//")
			continue;


		std::stringstream CurrentLineStream = std::stringstream(CurrentLine);

		//if the current line is empty, we ignore it
		if (!CurrentLine.empty())
		{

			std::string NativeType;
			CurrentLineStream >> NativeType;
			for (unsigned int i = 0; i < 8; i++)
			{
				if (NativeType::TypeStrings[i] == NativeType)
				{
					CurrentType = (NativeType::NativeType)((int)i);
				}
			}
			if (CurrentType == NativeType::Null)
			{
				Log::Print("Error reading material: " + NativeType + " is not a valid type (" + CurrentLine + ")", Vector3(1, 0, 0));
				return Material();
			}
			std::string Equals;

			CurrentLineStream >> CurrentName;
			CurrentLineStream >> Equals;

			if (Equals != "=")
			{
				Log::Print("Error reading material: expected = sign (" + CurrentLine + ")", Vector3(1, 0, 0));
				return Material();
			}
			//the rest of the stream is the value of the save item
			while (!CurrentLineStream.eof())
			{
				std::string ValueToAppend;
				CurrentLineStream >> ValueToAppend;
				Value.append(ValueToAppend + " ");
			}
			const auto strBegin = Value.find_first_not_of(" ");

			const auto strEnd = Value.find_last_not_of(" ");
			const auto strRange = strEnd - strBegin + 1;

			if (strRange > 0 && strBegin != std::string::npos)
			{
				Value = Value.substr(strBegin, strRange);
			}
			else
			{
				Value = "";
			}
			std::map<std::string, char*> PredifinedValues =
			{
				std::pair("UseShadowCutout", (char*)&OutMaterial.UseShadowCutout),
				std::pair("IsTranslucent", (char*)&OutMaterial.IsTranslucent),
			};

			if (PredifinedValues.contains(CurrentName))
			{
				SetPredefinedMaterialValue(Value, PredifinedValues[CurrentName], CurrentName);
			}
			else if (CurrentName == "VertexShader")
			{
				OutMaterial.VertexShader = Value;
			}
			else if (CurrentName == "FragmentShader")
			{
				OutMaterial.FragmentShader = Value;
			}
			else
			{
				OutMaterial.Uniforms.push_back(Param(CurrentName, CurrentType, Value, ""));
			}
		}
	}
	In.close();
	return OutMaterial;
}

void Material::SaveMaterialFile(std::string Path, Material m)
{
	std::ofstream Out = std::ofstream(Path);

	Out << "// Default material parameters:\n";
	Out << "string VertexShader = " << m.VertexShader << "\n";
	Out << "string FragmentShader = " << m.FragmentShader << "\n";
	Out << "int UseShadowCutout = " << std::to_string(m.UseShadowCutout) << "\n";
	Out << "int IsTranslucent = " << std::to_string(m.IsTranslucent) << "\n";
	Out << "// Material specific parameters:\n";

	for (const auto& Uniform : m.Uniforms)
	{
		if (Uniform.NativeType >= 0 && !Uniform.UniformName.empty())
		{
			Out << NativeType::TypeStrings.at(Uniform.NativeType) << " " << Uniform.UniformName << " = " << Uniform.Value << "\n";
		}
	}
	Out.close();
}

#if !SERVER
void Material::ReloadMaterial(std::string MaterialPath)
{
	Material NewMaterial = LoadMaterialFile(MaterialPath);
	for (Renderable* m : Graphics::MainFramebuffer->Renderables)
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