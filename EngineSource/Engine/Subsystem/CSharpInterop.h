#ifdef ENGINE_CSHARP
#pragma once
#include <string>
#include <Math/Vector.h>
#include <Engine/EngineError.h>
#include "Subsystem.h"

#if defined(_WIN32)
#define NET_CALLTYPE __stdcall
#else
#define NET_CALLTYPE
#endif


class WorldObject;

/**
* @brief
* File containing logic related to interoperability between C# and C++.
*/

class CSharpInterop : public Subsystem
{
public:

	static CSharpInterop* CSharpSystem;

	struct CSharpWorldObject
	{
		int32_t ID = 0;
	};

	void LoadRuntime();

	static void LoadAssembly();
	CSharpWorldObject InstantiateObject(std::string Typename, Transform t, WorldObject* NativeObject);
	void DestroyObject(CSharpWorldObject Obj);

	void* LoadCSharpFunction(std::string Function, std::string Namespace, std::string DelegateName);

	CSharpInterop();
	void Update() override;

	enum CSharpLogSev
	{
		CS_Log_Info = 0,
		CS_Log_Warn = 1,
		CS_Log_Err = 2,
	};

	enum CSharpLogType
	{
		CS_Log_Runtime,
		CS_Log_Build,
		CS_Log_Script
	};

	struct CSharpClass
	{
		std::string Name;
	};

	std::vector<std::string> GetAllClasses();

	static bool GetUseCSharp();

	static bool IsAssemblyLoaded();
	/**
	* @brief
	* Function used by C# code to print messages to the log.
	*/
	void CSharpLog(std::string Msg, CSharpLogType NativeType, CSharpLogSev Severity = CS_Log_Info);
	void RegisterNativeFunction(std::string Name, void* Function);

	void ExectuteFunctionOnObject(CSharpWorldObject Object, std::string FunctionName);
	std::string ExectuteStringFunctionOnObject(CSharpWorldObject Object, std::string FunctionName);

	std::string GetPropertyOfObject(CSharpWorldObject Object, std::string FunctionName);
	void SetPropertyOfObject(CSharpWorldObject Object, std::string FunctionName, std::string Value);

	Vector3 GetObjectVectorField(CSharpWorldObject Obj, std::string Field);
	void SetObjectVectorField(CSharpWorldObject Obj, std::string Field, Vector3 Value);

	static std::string GetNetVersion();

	void ReloadCSharpAssembly();

	template<typename T, typename... Args>
	static T StaticCall(void* Function, Args... argument)
	{
		ENGINE_ASSERT(Function, "Function cannot be null");
		typedef T(NET_CALLTYPE* f)(Args...);
		f fPtr = (f)Function;
		return fPtr(argument...);
	}
};
#endif