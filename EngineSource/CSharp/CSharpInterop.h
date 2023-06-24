#ifdef ENGINE_CSHARP
#pragma once
#include <string>
#include <Math/Vector.h>

class WorldObject;

namespace CSharp
{
	struct CSharpWorldObject
	{
		int32_t ID = 0;
	};

	void LoadRuntime();
	void UnloadRuntime();

	void LoadAssembly();
	CSharpWorldObject InstantiateObject(std::string Typename, Transform t, WorldObject* NativeObject);
	void DestroyObject(CSharpWorldObject Obj);

	void* LoadCSharpFunction(std::string Function, std::string Namespace, std::string DelegateName);

	void Init();
	void RunPerFrameLogic();

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

	bool IsAssemblyLoaded();
	void CSharpLog(std::string Msg, CSharpLogType Type, CSharpLogSev Severity = CS_Log_Info);
	void RegisterNativeFunction(std::string Name, void* Function);

	void ExectuteFunctionOnObject(CSharpWorldObject Object, std::string FunctionName);
	Vector3 GetObjectVectorField(CSharpWorldObject Obj, std::string Field);
	void SetObjectVectorField(CSharpWorldObject Obj, std::string Field, Vector3 Value);

	void ReloadCSharpAssembly();

	template<typename T, typename... Args> 
	T StaticCall(void* Function, Args... argument)
	{
		if (!Function)
		{
			return T();
		}
		typedef T(__stdcall* f)(Args...);
		f fPtr = (f)Function;
		return fPtr(argument...);
	}
}
#endif