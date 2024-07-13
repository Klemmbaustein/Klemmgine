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


class SceneObject;

/**
* @brief
* File containing logic related to interoperability between C# and C++.
*/

/**
* @brief
* Subsystem for managing interoperability between C# and C++
* 
* @ingroup Subsystem
*/
class CSharpInterop : public Subsystem
{
public:

	/// The current instance of the C# subsystem.
	static CSharpInterop* CSharpSystem;

	struct CSharpSceneObject
	{
		int32_t ID = 0;
	};

	void LoadRuntime();

	static void LoadAssembly();
	CSharpSceneObject InstantiateObject(std::string Typename, Transform t, SceneObject* NativeObject);
	void DestroyObject(CSharpSceneObject Obj);

	/**
	* @brief
	* Gets a pointer to a C# function.
	*
	* Only functions in CSharpCore.dll can be called.
	* 
	* @param Function
	* The name of the function.
	* 
	* @param Namespace
	* The namespace of the function.
	* 
	* @param DelegateName
	* The name of a C# delegate corresponding to the function defined in the Engine.Core.Delegates class.
	*/
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

	/**
	* @brief
	* Gets the names of all SceneObject classes defined in C#.
	*/
	std::vector<std::string> GetAllClasses();

	/**
	* @brief
	* Returns true if the project uses C# scripting, false if not.
	*/
	static bool GetUseCSharp();

	static bool IsAssemblyLoaded();
	/**
	* @brief
	* Function used by C# code to print messages to the log.
	*/
	void CSharpLog(std::string Msg, CSharpLogType NativeType, CSharpLogSev Severity = CS_Log_Info);
	
	/// Registers a native C++ function that can be called with Engine.Native.NativeFunction.CallNativeFunction() in C#.
	void RegisterNativeFunction(std::string Name, void* Function);

	/// Calls an empty void function with the given name on the given object.
	void ExecuteFunctionOnObject(CSharpSceneObject Object, std::string FunctionName);
	std::string ExecuteStringFunctionOnObject(CSharpSceneObject Object, std::string FunctionName);

	std::string GetPropertyOfObject(CSharpSceneObject Object, std::string FunctionName);
	void SetPropertyOfObject(CSharpSceneObject Object, std::string FunctionName, std::string Value);

	Vector3 GetObjectVectorField(CSharpSceneObject Obj, std::string Field);
	void SetObjectVectorField(CSharpSceneObject Obj, std::string Field, Vector3 Value);

	/// Gets the .NET version required or used.
	static std::string GetNetVersion();

	/// Reloads the game's C# code. Also reloads all C# objects.
	void ReloadCSharpAssembly();

	/**
	* @brief
	* Calls a static C# function.
	* 
	* @tparam T
	* The return value type of the managed function.
	* 
	* @tparam Args
	* The function argument types.
	* 
	* @param Function
	* A pointer to the managed function.
	* 
	* @param argument
	* The arguments for the managed function.
	* 
	* @return
	* The return value of the managed function.
	*/
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