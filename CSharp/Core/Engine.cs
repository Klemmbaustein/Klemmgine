using System;
using System.Runtime.InteropServices;
using System.Reflection;
using System.IO;
using System.Globalization;
using System.Text.RegularExpressions;

[StructLayout(LayoutKind.Sequential)]
public struct EngineVector
{
	public float X = 0;
	public float Y = 0;
	public float Z = 0;

	public EngineVector()
	{
	}
	public EngineVector(float x, float y, float z)
	{
		X = x;
		Y = y;
		Z = z;
	}
}
[StructLayout(LayoutKind.Sequential)]
public struct EngineTransform
{
	public EngineVector Position;
	public EngineVector Rotation;
	public EngineVector Scale;

	public EngineTransform()
	{
		Position = new EngineVector();
		Rotation = new EngineVector();
		Scale = new EngineVector();
	}
	public EngineTransform(EngineVector x, EngineVector y, EngineVector z)
	{
		Position = x;
		Rotation = y;
		Scale = z;
	}
}

static class Engine
{
	public static Assembly? LoadedAsm = null;
	static Int64 CurrentObjectIndex = 0;
	static Type? StatsObject = null;
	static Type? InputObject = null;

	static readonly Dictionary<Int32, Object> SceneObjects = [];
	static readonly List<Type> SceneObjectTypes = [];

	public static object? GetObjectFromID(Int32 Id)
	{
		if (SceneObjects.TryGetValue(Id, out object? value))
		{
			return value;
		}
		return null;
	}

	public static object? GetObjectFromPtr(IntPtr Ptr)
	{
		foreach (var i in SceneObjects)
		{
			if ((IntPtr)Get(i.Value, "NativePtr")! == Ptr)
			{
				return i.Value;
			}
		}
		return null;
	}

	static bool LoadedEngine = false;

	public static void LoadAssembly([MarshalAs(UnmanagedType.LPUTF8Str)] string Path, [MarshalAs(UnmanagedType.LPUTF8Str)] string EngineDllPath, int EngineConfig)
	{
		SceneObjectTypes.Clear();
		CoreNativeFunction.UnloadNativeFunctions();
		EngineLog.Print("Loading C# assembly...");

		if (!LoadedEngine)
		{
			Assembly.LoadFrom(EngineDllPath);
			LoadedEngine = true;
		}
		else
		{
			LoadTypeFromAssembly("Engine.Native.NativeFunction")?.GetMethod("UnloadAll")?.Invoke(null, null);
		}
		LoadedAsm = Assembly.Load(File.ReadAllBytes(Path));
		LoadTypeFromAssembly("Engine.Log")!.GetMethod("LoadLogFunction")!.Invoke(null, [new Action<string, int>(EngineLog.Print)]);


		var SceneObjectType = LoadTypeFromAssembly("Engine.SceneObject");

		if (SceneObjectType == null)
		{
			EngineLog.Print("Could not load SceneObject class.");
			return;
		}

		SceneObjectType.GetMethod("LoadGetObjectFunctions")!.Invoke(null, new object[] 
		{ 
			(object)GetObjectFromID,
			(object)GetObjectFromPtr
		});

		foreach (var i in LoadedAsm.GetTypes())
		{
			if (i.IsSubclassOf(SceneObjectType!))
			{
				SceneObjectTypes.Add(i);
			}
		}

		StatsObject = LoadTypeFromAssembly("Engine.Stats");
		StatsObject!.GetField("Config")!.SetValue(null, EngineConfig);
		InputObject = LoadTypeFromAssembly("Engine.Input");
	}

	public static Int32 Instantiate(string obj, EngineTransform t, IntPtr NativeObject)
	{
		foreach (var ObjectType in SceneObjectTypes)
		{
			if (ObjectType.Name == obj)
			{
				object NewObject = Activator.CreateInstance(ObjectType)!;
				if (NewObject == null)
				{
					return 0;
				}

				CurrentObjectIndex++;
				Int32 HashCode = CurrentObjectIndex.GetHashCode();
				if (HashCode == 0)
				{
					HashCode = -1;
				}
				Set(ref NewObject!, "NativePtr", NativeObject);

				SceneObjects.Add(HashCode, NewObject);
				SetVectorFieldOfObject(HashCode, "Position", t.Position);
				SetVectorFieldOfObject(HashCode, "Rotation", t.Rotation);
				SetVectorFieldOfObject(HashCode, "Scale", t.Scale);
				return HashCode;
			}
		}
		return 0;
	}

	public static void Destroy(Int32 ID)
	{
		if (!SceneObjects.ContainsKey(ID))
		{
			return;
		}

		ExecuteFunctionOnObject(ID, "DestroyObjectInternal");
		SceneObjects.Remove(ID);
	}

	public static Type? LoadTypeFromAssembly(string Type)
	{
		if (LoadedAsm == null)
		{
			EngineLog.Print("Tried to call method while the C# assembly is unloaded!", 2);
			return null;
		}

		foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies().Reverse())
		{
			var tt = assembly.GetType(Type);
			if (tt != null)
			{
				return tt;
			}
		}
		EngineLog.Print("Failed to load type " + Type, 2);
		return null;
	}

	public static object? SafeInvokeMethod(MethodInfo Method, object? TargetObject, object?[]? args)
	{
		try
		{
			return Method.Invoke(TargetObject, args);
		}
		catch (Exception e)
		{
			EngineLog.Print(e.InnerException!.ToString(), 2);
			return null;
		}
	}

	public static void ExecuteFunctionOnObject(Int32 ID, [MarshalAs(UnmanagedType.LPUTF8Str)] string FunctionName)
	{
		if (!SceneObjects.TryGetValue(ID, out object? value))
		{
			EngineLog.Print(string.Format("Tried to call {0} on the object with ID {1} but that object doesn't exist!", FunctionName, ID), 1);
			return;
		}
		var obj = value;
		var func = obj.GetType().GetMethod(FunctionName, BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Public);
		if (func == null)
		{
			EngineLog.Print(string.Format("Tried to call {0} on {1} but that function doesn't exist on this class!", FunctionName, obj.GetType().Name), 1);
			return;
		}
		SafeInvokeMethod(func, obj, null);
	}

	[return: MarshalAs(UnmanagedType.LPUTF8Str)]
	public static string ExecuteStringFunctionOnObject(Int32 ID, [MarshalAs(UnmanagedType.LPUTF8Str)] string FunctionName)
	{
		if (!SceneObjects.TryGetValue(ID, out object? value))
		{
			EngineLog.Print(string.Format("Tried to call {0} on the object with ID {1} but that object doesn't exist!", FunctionName, ID), 1);
			return "";
		}
		var obj = value;
		var func = obj.GetType().GetMethod(FunctionName, BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Public);
		if (func == null)
		{
			EngineLog.Print(string.Format("Tried to call {0} on {1} but that function doesn't exist on this class!", FunctionName, obj.GetType().Name), 1);
			return "";
		}
		return (string)SafeInvokeMethod(func, obj, [])!;
	}

	[return: MarshalAs(UnmanagedType.LPUTF8Str)]
	public static string GetObjectPropertyString(Int32 ID, [MarshalAs(UnmanagedType.LPUTF8Str)] string PropertyName)
	{
		if (!SceneObjects.TryGetValue(ID, out object? value))
		{
			EngineLog.Print(string.Format("Tried get the field {0} on the object with ID {1} but that object doesn't exist!", PropertyName, ID));
			return "";
		}
		var obj = value;
		var field = obj.GetType().GetField(PropertyName, BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance);
		if (field == null)
		{
			EngineLog.Print(string.Format("Tried to get the field {0} on {1} but that field doesn't exist on this class!", PropertyName, obj.GetType().Name));
			return "";
		}

		if (!field.FieldType.IsArray)
		{
			return field.GetValue(obj)?.ToString()!;
		}
		object[] arr = (object[])field.GetValue(obj)!;
		string str = "";
		foreach (object o in arr)
		{
			str += o.ToString() + "\r";
		}
		return str;
	}

	public static void SetObjectPropertyString(Int32 ID,
		[MarshalAs(UnmanagedType.LPUTF8Str)] string PropertyName,
		[MarshalAs(UnmanagedType.LPUTF8Str)] string PropertyValue)
	{
		if (!SceneObjects.TryGetValue(ID, out object? value))
		{
			EngineLog.Print(string.Format("Tried set the field {0} on the object with ID {1} but that object doesn't exist!", PropertyName, ID), 1);
			return;
		}
		var obj = value;
		var field = obj.GetType().GetField(PropertyName, BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance);
		if (field == null)
		{
			EngineLog.Print(string.Format("Tried to set the field {0} on {1} but that field doesn't exist on this class!", PropertyName, obj.GetType().Name), 1);
			return;
		}

		if (field.FieldType.IsArray)
		{
			return;
			// TODO: Implement property arrays
			/*string[] entries = PropertyValue.Split(new string[] { "\r" }, StringSplitOptions.None).Select(x => x).ToArray();

			if (entries.Last().Length == 0)
			{
				entries = entries.SkipLast(1).ToArray();
			}


			object[] arr = (object[])field.GetValue(obj)!;
			Array destinationArray = Array.CreateInstance(arr[0].GetType(), arr.Length);
			field.SetValue(obj, destinationArray);
			for (int i = 0; i < arr.Length; i++)
			{
				switch (field.FieldType.ToString())
				{
					case "System.Int[]":
						if (int.TryParse(PropertyValue, out int iResult))
						{
							destinationArray.SetValue(iResult, i);
						}
						break;
					case "System.Float[]":
						if (float.TryParse(PropertyValue, out float fResult))
						{
							destinationArray.SetValue(fResult, i);
						}
						break;
					case "System.String[]":
						destinationArray.SetValue(PropertyValue, i);
						break;
					case "Engine.Vector3[]":
						var Culture = CultureInfo.GetCultureInfo("en-US");
						float[] newPosCoordinates = PropertyValue.Split(new string[] { " " }, StringSplitOptions.None).Select(x => float.Parse(x, Culture)).ToArray();
						var vec = destinationArray.GetValue(i)!;
						Set(ref vec, "X", newPosCoordinates[0]);
						Set(ref vec, "Y", newPosCoordinates[1]);
						Set(ref vec, "Z", newPosCoordinates[2]);
						destinationArray.SetValue(vec, i);
						break;
					default:
						EngineLog.Print("Unknown type: " + field.FieldType.ToString());
						break;
				}
			}
			EngineLog.Print(destinationArray.Length.ToString());
			return;*/
		}

		switch (field.FieldType.ToString())
		{
			case "System.Int":
				if (int.TryParse(PropertyValue, out int iResult))
				{
					field.SetValue(obj, iResult);
				}
				break;
			case "System.Single":
			case "System.Float":
				if (float.TryParse(PropertyValue, out float fResult))
				{
					field.SetValue(obj, fResult);
				}
				break;
			case "System.String":
				field.SetValue(obj, PropertyValue);
				break;
			case "Engine.Vector3":
				var Culture = CultureInfo.GetCultureInfo("en-US");
				float[] newPosCoordinates = PropertyValue.Split([ " " ], StringSplitOptions.None).Select(x => float.Parse(x, Culture)).ToArray();
				var vec = field.GetValue(obj)!;
				Set(ref vec, "X", newPosCoordinates[0]);
				Set(ref vec, "Y", newPosCoordinates[1]);
				Set(ref vec, "Z", newPosCoordinates[2]);
				field.SetValue(obj, vec);
				break;
			case "System.Boolean":
				field.SetValue(obj, PropertyValue == "True");
				break;
			default:
				EngineLog.Print("Unknown type: " + field.FieldType.ToString(), 2);
				break;
		}
		obj.GetType()!.GetMethod("OnPropertySet")!.Invoke(obj, []);
	}

	public static object? Get(object obj, string member)
	{
		var MemberField = obj.GetType().GetField(member);
		return MemberField?.GetValue(obj);
	}
	public static void Set(ref object obj, string field, object value)
	{
		var MemberField = obj?.GetType().GetField(field);
		if (MemberField == null)
		{
			EngineLog.Print("Object " + obj?.ToString() + " does not have member " + field);
			return;
		}
		MemberField?.SetValueDirect(__makeref(obj), value);
	}

	public static EngineVector GetVectorFieldOfObject(Int32 ID, [MarshalAs(UnmanagedType.LPUTF8Str)] string Field)
	{
		if (!SceneObjects.TryGetValue(ID, out object? value))
		{
			EngineLog.Print(string.Format("Tried to access {1} of object with ID {0} but that object doesn't exist!", ID, Field));
			return new EngineVector();
		}
		var obj = value;
		var pos = Get(obj, Field);

		return new EngineVector((float)Get(pos!, "X")!, (float)Get(pos!, "Y")!, (float)Get(pos!, "Z")!);
	}

	public static void SetVectorFieldOfObject(Int32 ID, [MarshalAs(UnmanagedType.LPUTF8Str)] string Field, EngineVector NewValue)
	{
		if (!SceneObjects.ContainsKey(ID))
		{
			EngineLog.Print(string.Format("Tried to access {1} of object with ID {0} but that object doesn't exist!", ID, Field));
			return;
		}
		var obj = SceneObjects[ID];
		if (obj == null)
		{
			return;
		}
		var pos = obj?.GetType()?.GetField(Field)?.GetValue(obj)!;
		if (pos == null)
		{
			return;
		}
		Set(ref pos, "X", NewValue.X);
		Set(ref pos, "Y", NewValue.Y);
		Set(ref pos, "Z", NewValue.Z);
		obj?.GetType()?.GetField(Field)?.SetValue(obj, pos);
	}

	public static void SetDelta(float NewDelta)
	{
		StatsObject!.GetField("DeltaTime")?.SetValue(null, NewDelta);
		SafeInvokeMethod(InputObject!.GetMethod("UpdateGamepadList")!, null, null);
	}

	[return: MarshalAs(UnmanagedType.LPUTF8Str)]
	public static string GetAllObjectTypes()
	{
		string AllTypes = "";
		foreach (var i in SceneObjectTypes)
		{
			if (i.Namespace != null)
			{
				AllTypes += i.Namespace.Replace('.', '/') + "/" + i.Name + " ";
			}
			else
			{
				AllTypes += i.Name + " ";
			}
		}

		return AllTypes;
	}

	[return: MarshalAs(UnmanagedType.LPUTF8Str)]
	public static string GetStartupSceneInternally()
	{
		var ProjectType = LoadTypeFromAssembly("Project");
		if (ProjectType == null)
		{
			EngineLog.Print("Failed to load project type", 2);
			return "";
		}
		var StartupScene = ProjectType.GetMethod("GetStartupScene");
		if (StartupScene == null)
		{
			EngineLog.Print("Failed to load Project.GetStartupScene()", 2);
			return "";
		}
		return (string)SafeInvokeMethod(StartupScene, null, null)!;
	}

	public static void OnLaunchInternally()
	{
		var ProjectType = LoadTypeFromAssembly("Project");
		if (ProjectType == null)
		{
			EngineLog.Print("Failed to load project type", 2);
			return;
		}
		var OnLaunch = ProjectType.GetMethod("OnLaunch");
		if (OnLaunch == null)
		{
			EngineLog.Print("Failed to load Project.OnLaunch()", 2);
			return;
		}
		SafeInvokeMethod(OnLaunch, null, null);
		return;
	}

	public static string GetNameInternally()
	{
		var ProjectType = LoadTypeFromAssembly("Project");
		if (ProjectType == null)
		{
			EngineLog.Print("Failed to load project type", 2);
			return "";
		}
		var GetProjectName = ProjectType.GetMethod("GetProjectName");
		if (GetProjectName == null)
		{
			EngineLog.Print("Failed to load Project.GetProjectName()", 2);
			return "";
		}
		return (string)SafeInvokeMethod(GetProjectName, null, null)!;
	}

}
