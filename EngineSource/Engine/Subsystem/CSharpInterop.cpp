#ifdef ENGINE_CSHARP
#include "CSharpInterop.h"
#include <CSharp/NativeFunctions.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <iostream>
#include <Engine/EngineError.h>
#include <Engine/Log.h>
#include <Engine/Stats.h>
#include <Objects/CSharpObject.h>
#include <Engine/Build/Build.h>
#include <Engine/File/SaveData.h>
#include <Engine/Subsystem/Console.h>
#include <Engine/OS.h>
#include <Engine/Application.h>

#include <Utility/DotNet/nethost.h>
#include <Utility/DotNet/coreclr_delegates.h>
#include <Utility/DotNet/hostfxr.h>

#define CSHARP_LIBRARY_NAME "CSharpCore"

#if !RELEASE
#define CSHARP_LIBRARY_PATH STR("CSharp/Core/Build/CSharpCore")
#else
#define CSHARP_LIBRARY_PATH STR("bin/CSharp/Core/CSharpCore")
#endif

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>

#define STR(s) L ## s
#define CH(c) L ## c
#define DIR_SEPARATOR L'\\'

#else
#include <dlfcn.h>
#include <limits.h>

#define STR(s) s
#define CH(c) c
#define DIR_SEPARATOR '/'
#define MAX_PATH PATH_MAX

#endif

#if _WIN32
#define fs_string() wstring()
#else
#define fs_string() string()
#endif

CSharpInterop* CSharpInterop::CSharpSystem = nullptr;


namespace CSharp
{
	bool UseCSharp = false;
	bool LoadedUseCSharp = false;

	std::string CSharpLogTypes[3] =
	{
		"Runtime",
		"Build",
		"Script"
	};

	Vector3 CSharpLogColors[3] =
	{
		Log::LogColor::White,
		Log::LogColor::Yellow,
		Log::LogColor::Red
	};
	void* ExecuteOnObjectFunction = nullptr;
	void* ExecuteStringOnObjectFunction = nullptr;
	void* GetPropertyFunction = nullptr;
	void* SetPropertyFunction = nullptr;
	void* InstantiateFunction = nullptr;
	void* DestroyFunction = nullptr;
	void* GetPosFunction = nullptr;
	void* SetPosFunction = nullptr;
	void* SetDeltaFunction = nullptr;
	void* RegisterNativeFunctionPtr = nullptr;
	void* GetAllClassesPtr = nullptr;

	std::vector<std::string> AllClasses;
}

using namespace CSharp;
using string_t = std::basic_string<char_t>;

namespace CSharp
{	
	// Globals to hold hostfxr exports
	hostfxr_initialize_for_runtime_config_fn init_fptr;
	hostfxr_get_runtime_delegate_fn get_delegate_fptr;
	hostfxr_close_fn close_fptr;
	hostfxr_handle cxt = nullptr;

	void* hostfxr_lib = nullptr;

	// Great name
	load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;

	load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t* assembly);

#ifdef _WIN32
	static void* load_library(const char_t* path)
	{
		HMODULE h = ::LoadLibraryW(path);
		ENGINE_ASSERT(h != nullptr, "Could not load library");
		return (void*)h;
	}
	static void* get_export(void* h, const char* name)
	{
		void* f = ::GetProcAddress((HMODULE)h, name);
		ENGINE_ASSERT(f != nullptr, "Could not export library: " + std::string(name));
		return f;
	}
#else
	static void* load_library(const char_t* path)
	{
		void* h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
		ENGINE_ASSERT(h != nullptr, "Could not load library.");
		return h;
	}
	static void* get_export(void* h, const char* name)
	{
		void* f = dlsym(h, name);
		ENGINE_ASSERT(f != nullptr, "Could not export library");
		return f;
	}

#endif

	// Using the nethost library, discover the location of hostfxr and get exports
	static bool load_hostfxr()
	{
		if (!CSharpInterop::GetUseCSharp())
		{
			return false;
		}
		// Pre-allocate a large buffer for the path to hostfxr
		char_t buffer[MAX_PATH];
		size_t buffer_size = sizeof(buffer) / sizeof(char_t);
#if RELEASE && _WIN32
		string_t Path = std::filesystem::current_path().fs_string() + STR("/bin");
		string_t NetPath = Path + STR("/NetRuntime");

		get_hostfxr_parameters parameters = 
		{
			sizeof(hostfxr_initialize_parameters),
			nullptr,
			string_t(NetPath.begin(), NetPath.end()).c_str()
		};

		int rc = get_hostfxr_path(buffer, &buffer_size, &parameters);
#else
		int rc = get_hostfxr_path(buffer, &buffer_size, nullptr);
#endif
		if (rc != 0)
		{
			Log::Print("Could not get hostfxr path - Error code: " + std::to_string(rc) + " : " + std::to_string(buffer_size), Log::LogColor::Red);
			return false;
		}
#if _WIN32
		CSharpInterop::CSharpSystem->CSharpLog("Using .net runtime .dll: '" + FileUtil::wstrtostr(buffer) + "'", CSharpInterop::CS_Log_Runtime);
#else
		CSharpInterop::CSharpSystem->CSharpLog("Using .net runtime .so: '" + std::string(buffer) + "'", CSharpInterop::CS_Log_Runtime);
#endif
		// Load hostfxr and get desired exports
		hostfxr_lib = load_library(buffer);
		init_fptr = (hostfxr_initialize_for_runtime_config_fn)get_export(hostfxr_lib, "hostfxr_initialize_for_runtime_config");
		get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)get_export(hostfxr_lib, "hostfxr_get_runtime_delegate");
		close_fptr = (hostfxr_close_fn)get_export(hostfxr_lib, "hostfxr_close");

		return (init_fptr && get_delegate_fptr && close_fptr);
	}

	// Load and initialize .NET Core and get desired function pointer for scenario
	load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t* config_path)
	{
		// Load .NET Core
		void* load_assembly_and_get_function_pointer = nullptr;

		// For some unknown reason this doesn't work on linux.
#if RELEASE && _WIN32
		string_t Path = std::filesystem::current_path().fs_string() + STR("/bin");
		string_t NetPath = Path + STR("/NetRuntime");

		hostfxr_initialize_parameters parameters = 
		{
			sizeof(hostfxr_initialize_parameters),
			string_t(Path.begin(), Path.end()).c_str(),
			string_t(NetPath.begin(), NetPath.end()).c_str()
		};

		int rc = init_fptr(config_path, &parameters, &cxt);
#else
		int rc = init_fptr(config_path, nullptr, &cxt);
#endif
		if (rc != 0 || cxt == nullptr)
		{
			CSharpInterop::CSharpSystem->CSharpLog("Init failed", CSharpInterop::CS_Log_Runtime, CSharpInterop::CS_Log_Err);
			close_fptr(cxt);
			return nullptr;
		}

		// Get the load assembly function pointer
		rc = get_delegate_fptr(
			cxt,
			hdt_load_assembly_and_get_function_pointer,
			&load_assembly_and_get_function_pointer);
		if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
			std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;

		close_fptr(cxt);
		return (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;
	}
}


static void WriteCSProj(std::string Name)
{
	std::filesystem::create_directories("CSharp/Build");
	std::ofstream out = std::ofstream(Name);
	out << "<Project Sdk=\"Microsoft.NET.Sdk\">\n\
	<PropertyGroup>\n\
		<TargetFramework>net" + CSharpInterop::GetNetVersion() + ".0</TargetFramework>\n\
		<EnableDynamicLoading>true</EnableDynamicLoading>\n\
	</PropertyGroup>\n\
	<PropertyGroup>\n\
		<OutputPath>" + std::filesystem::current_path().string() + "/CSharp/Build</OutputPath>\n\
		<AppendTargetFrameworkToOutputPath>false</AppendTargetFrameworkToOutputPath>\n\
	</PropertyGroup>\n\
  <ItemGroup>\n\
	<Reference Include=\"KlemmgineCSharp.dll\">\n\
	  <HintPath>" + Application::GetEditorPath() + "/CSharp/Engine/Build/KlemmgineCSharp.dll</HintPath>\n\
	</Reference>\n\
  </ItemGroup>\n\
</Project>";
	out.close();
}
static void CSharpInternalPrint(const char* Msg, int Severity)
{
	CSharpInterop::CSharpSystem->CSharpLog(Msg, CSharpInterop::CS_Log_Script, (CSharpInterop::CSharpLogSev)Severity);
}

CSharpInterop::CSharpInterop()
{
	CSharpSystem = this;
	// CSharp written out instead of C# because it looks better in the log.
	Name = "CSharp";
	if (!GetUseCSharp())
	{
		return;
	}

#if !RELEASE
	Console::ConsoleSystem->RegisterCommand(Console::Command("makecsproj", []() 
		{
			CSharpInterop::CSharpSystem->CSharpLog("Rebuilding project csproj file...", CS_Log_Build);
			WriteCSProj("Scripts/CSharpAssembly.csproj");
		}, {}));
	if (!std::filesystem::exists("Scripts/CSharpAssembly.csproj"))
	{
		Console::ExecuteConsoleCommand("makecsproj");
	}
#endif
	LoadRuntime();
	void* LogRegister = LoadCSharpFunction("LoadLogFunction", "EngineLog", "LoadFunctionDelegate");

	StaticCall<void, void*>(LogRegister, (void*)CSharpInternalPrint);

	GetPropertyFunction = LoadCSharpFunction("GetObjectPropertyString", "Engine", "ExecuteOnStringDelegate");
	SetPropertyFunction = LoadCSharpFunction("SetObjectPropertyString", "Engine", "SetPropertyDelegate");
	ExecuteOnObjectFunction = LoadCSharpFunction("ExecuteFunctionOnObject", "Engine", "ExecuteOnDelegate");
	ExecuteStringOnObjectFunction = LoadCSharpFunction("ExecuteStringFunctionOnObject", "Engine", "ExecuteOnStringDelegate");
	InstantiateFunction = LoadCSharpFunction("Instantiate", "Engine", "InstantiateDelegate");
	GetPosFunction = LoadCSharpFunction("GetVectorFieldOfObject", "Engine", "GetVectorDelegate");
	SetPosFunction = LoadCSharpFunction("SetVectorFieldOfObject", "Engine", "SetVectorDelegate");
	DestroyFunction = LoadCSharpFunction("Destroy", "Engine", "VoidInt32InDelegate");
	SetDeltaFunction = LoadCSharpFunction("SetDelta", "Engine", "VoidFloatInDelegate");
	RegisterNativeFunctionPtr = LoadCSharpFunction("RegisterNativeFunction", "CoreNativeFunction", "LoadNativeFunctionDelegate");
	GetAllClassesPtr = LoadCSharpFunction("GetAllObjectTypes", "Engine", "StringDelegate");

	LoadAssembly();
	NativeFunctions::RegisterNativeFunctions();
}

void CSharpInterop::Update()
{
	if (!GetUseCSharp())
	{
		return;
	}

	CSharpInterop::StaticCall<void, float>(SetDeltaFunction, Stats::DeltaTime);
}

std::vector<std::string> CSharpInterop::GetAllClasses()
{
	return AllClasses;
}

bool CSharpInterop::GetUseCSharp()
{
#if ENGINE_NO_SOURCE
	return true;
#endif
	if (!LoadedUseCSharp)
	{
		LoadedUseCSharp = true;
#if RELEASE
		UseCSharp = std::filesystem::exists("bin/CSharp");
#else
		SaveData g = SaveData(Build::GetProjectBuildName(), "keproj", false);
		if (g.SaveGameIsNew())
		{
			UseCSharp = true;
		}
		else
		{
			UseCSharp = g.GetBool("C#:Use_C#_in_project_(Requires_restart)");
		}
#endif
		return UseCSharp;
	}
	else
	{
		return UseCSharp;
	}
}
bool CSharpInterop::IsAssemblyLoaded()
{
	return CSharp::hostfxr_lib;
}

void CSharpInterop::CSharpLog(std::string Msg, CSharpLogType NativeType, CSharpLogSev Severity)
{
	Log::PrintMultiLine(Msg, CSharp::CSharpLogColors[Severity], "[C#]: [" + CSharp::CSharpLogTypes[NativeType] + "]: ");
}

void CSharpInterop::RegisterNativeFunction(std::string Name, void* Function)
{
	StaticCall<void, const char*, void*>(RegisterNativeFunctionPtr, Name.c_str(), Function);
}

void CSharpInterop::ExecuteFunctionOnObject(CSharpSceneObject Object, std::string FunctionName)
{
	StaticCall<void, int32_t, const char*>(ExecuteOnObjectFunction, Object.ID, FunctionName.c_str());
}

std::string CSharpInterop::ExecuteStringFunctionOnObject(CSharpSceneObject Object, std::string FunctionName)
{
	return StaticCall<const char*, int32_t, const char*>(ExecuteStringOnObjectFunction, Object.ID, FunctionName.c_str());
}

std::string CSharpInterop::GetPropertyOfObject(CSharpSceneObject Object, std::string FunctionName)
{
	return StaticCall<const char*, int32_t, const char*>(GetPropertyFunction, Object.ID,  FunctionName.c_str());
}

void CSharpInterop::SetPropertyOfObject(CSharpSceneObject Object, std::string FunctionName, std::string Value)
{
	return StaticCall<void, int32_t, const char*, const char*>(SetPropertyFunction, Object.ID, FunctionName.c_str(), Value.c_str());
}

Vector3 CSharpInterop::GetObjectVectorField(CSharpSceneObject Obj, std::string Field)
{
	return StaticCall<Vector3, int32_t, const char*>(GetPosFunction, Obj.ID, Field.c_str());
}

void CSharpInterop::SetObjectVectorField(CSharpSceneObject Obj, std::string Field, Vector3 Value)
{
	return StaticCall<void, int32_t, const char*, Vector3>(SetPosFunction, Obj.ID, Field.c_str(), Value);
}

std::string CSharpInterop::GetNetVersion()
{
	return "8";
}

void CSharpInterop::ReloadCSharpAssembly()
{
	if (!GetUseCSharp())
	{
		return;
	}
	LoadAssembly();
	NativeFunctions::RegisterNativeFunctions();
	for (auto i : Objects::GetAllObjectsWithID(CSharpObject::GetID()))
	{
		dynamic_cast<CSharpObject*>(i)->Reload(false);
	}
}

void CSharpInterop::LoadRuntime()
{
	if (!GetUseCSharp())
	{
		return;
	}
	CSharpLog("Loading .net runtime...", CS_Log_Runtime);
	string_t root_path = std::filesystem::current_path().fs_string();
	auto pos = root_path.find_last_of(DIR_SEPARATOR);
	ENGINE_ASSERT(pos != string_t::npos, "Root path isn't valid");

	ENGINE_ASSERT(load_hostfxr(), "Failure: load_hostfxr() failed.");

	if (Application::GetEditorPath() != std::filesystem::current_path().string())
	{
#if !RELEASE
#if _WIN32
		root_path = OS::Utf8ToWstring(Application::GetEditorPath());
#else
		root_path = Application::GetEditorPath();
#endif
#endif
	}

#if !RELEASE
	const string_t config_path = std::filesystem::absolute(root_path).fs_string() + STR("/") + string_t(CSHARP_LIBRARY_PATH) + STR(".runtimeconfig.json");
#else
	const string_t config_path = root_path + STR("/") + string_t(CSHARP_LIBRARY_PATH) + STR(".runtimeconfig.json");
#endif
	load_assembly_and_get_function_pointer = get_dotnet_load_assembly(config_path.c_str());
	if (load_assembly_and_get_function_pointer == nullptr)
	{
		CSharpLog("Could not load .net runtime!", CS_Log_Runtime, CS_Log_Err);
		return;
	}
}

void* CSharpInterop::LoadCSharpFunction(std::string Function, std::string Namespace, std::string DelegateName)
{
	if (!GetUseCSharp())
	{
		return nullptr;
	}
	typedef void (CORECLR_DELEGATE_CALLTYPE* StaticFunction)();
	StaticFunction fCallback = nullptr;

	string_t root_path = std::filesystem::current_path().fs_string();
	auto pos = root_path.find_last_of(DIR_SEPARATOR);
	ENGINE_ASSERT(pos != string_t::npos, "Root path isn't valid");

	if (Application::GetEditorPath() != std::filesystem::current_path().string())
	{
#if !RELEASE
#if _WIN32
		root_path = OS::Utf8ToWstring(Application::GetEditorPath());
#else
		root_path = Application::GetEditorPath();
#endif
#endif
	}

	const string_t dotnetlib_path = root_path + STR("/") + string_t(CSHARP_LIBRARY_PATH) + STR(".dll");
	std::string Name = CSHARP_LIBRARY_NAME;

	string_t dotnet_type = string_t(Namespace.begin(), Namespace.end()) + STR(", ") + string_t(Name.begin(), Name.end());

	int rc = load_assembly_and_get_function_pointer(
		dotnetlib_path.c_str(),
		dotnet_type.c_str(),
		string_t(Function.begin(), Function.end()).c_str(),
		(STR("Delegates+") + string_t(DelegateName.begin(), DelegateName.end()) + STR(", ") + string_t(Name.begin(), Name.end())).c_str(),
		nullptr,
		(void**)&fCallback);


	if (rc || !fCallback)
	{
		CSharpLog("Failed to load function: " + Namespace + "." + Function, CS_Log_Runtime, CS_Log_Err);
		return nullptr;
	}

	return (void*)fCallback;
}

void CSharpInterop::LoadAssembly()
{
	if (!GetUseCSharp())
	{
		return;
	}

	// Engine.Stats.EngineConfig enum for more information.
	int Config = 0;
#if EDITOR
	Config = 0;
#elif RELEASE
	Config = 2;
#elif SERVER
	Config = 3;
#else
	Config = 1;
#endif

#if !RELEASE

	std::string AssemblyPath = std::filesystem::current_path().string() + "/CSharp/Build/CSharpAssembly.dll";

	if (!std::filesystem::exists(AssemblyPath))
	{
		CSharpSystem->CSharpLog("Could not find C# assembly. Attempting rebuild.", CS_Log_Runtime, CS_Log_Warn);
		CSharpSystem->CSharpLog("------------------------------------------------", CS_Log_Runtime, CS_Log_Warn);
		system("cd Scripts && dotnet build");
	}
	if (!std::filesystem::exists(AssemblyPath))
	{
		CSharpSystem->CSharpLog("Could not find or rebuild C# assembly.", CS_Log_Runtime, CS_Log_Err);
		return;
	}

	try
	{
		std::filesystem::copy(Application::GetEditorPath() + "/CSharp/Engine/Build/KlemmgineCSharp.dll",
			"bin/KlemmgineCSharp.dll",
			std::filesystem::copy_options::update_existing);
	}
	catch (std::exception& e)
	{
		Log::Print(e.what(), Log::LogColor::Red);
	}


	StaticCall<void, const char*, const char*, int>(CSharpSystem->LoadCSharpFunction("LoadAssembly", "Engine", "LoadAssemblyDelegate"),
		AssemblyPath.c_str(), "bin/KlemmgineCSharp.dll", Config);
#else
	StaticCall<void, const char*, const char*, int>(CSharpSystem->LoadCSharpFunction("LoadAssembly", "Engine", "LoadAssemblyDelegate"),
		(std::filesystem::current_path().string() + "/bin/CSharp/CSharpAssembly.dll").c_str(),
		(std::filesystem::current_path().string() + "/bin/CSharp/Engine/KlemmgineCSharp.dll").c_str(),
		Config);
#endif

	AllClasses.clear();
	std::stringstream AllClassesStream;
	AllClassesStream << StaticCall<const char*>(GetAllClassesPtr);
	while (!AllClassesStream.eof())
	{
		std::string NewClass;
		AllClassesStream >> NewClass;
		if (!NewClass.empty())
		{
			AllClasses.push_back(NewClass);
		}
	}
}

CSharpInterop::CSharpSceneObject CSharpInterop::InstantiateObject(std::string Typename, Transform t, SceneObject* NativeObject)
{
	if (!GetUseCSharp())
	{
		return CSharpSceneObject();
	}
	return CSharpSceneObject(StaticCall<int32_t, const char*, Transform, void*>(InstantiateFunction, Typename.c_str(), t, NativeObject));
}

void CSharpInterop::DestroyObject(CSharpSceneObject Obj)
{
	if (!GetUseCSharp())
	{
		return;
	}
	StaticCall<void, int32_t>(DestroyFunction, Obj.ID);
}

#endif