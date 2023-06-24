using System;

public static class Delegates
{
	public delegate void VoidDelegate();
	public delegate string StringDelegate();

	public delegate void VoidStringInDelegate(string In);
	public delegate void VoidFloatInDelegate(float In);
	public delegate void VoidInt32InDelegate(Int32 In);

	public delegate void LoadAssemblyDelegate(string AsmPath, bool InEditor);
	public delegate void LoadFunctionDelegate(IntPtr LoadedFunction);
	public delegate void LoadNativeFunctionDelegate(string Name, IntPtr FunctionPtr);
	public delegate Int32 InstantiateDelegate(string obj, EngineTransform t, IntPtr NativeObject);
	public delegate void ExecuteOnDelegate(Int32 Object, string FunctionName);
	public delegate EngineVector GetVectorDelegate(Int32 Object, string Field);
	public delegate void SetVectorDelegate(Int32 Object, string Field, EngineVector NewVec);

}