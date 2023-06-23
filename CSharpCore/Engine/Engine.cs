using System;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Security.Cryptography;
#nullable enable

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

internal static class Engine
{
	public static Assembly? LoadedAsm = null;
	static Int64 CurrentObjectIndex = 0;
	static Type? StatsObject = null;

	static Dictionary<Int32, Object> WorldObjects = new Dictionary<Int32, Object>();
	static List<Type> WorldObjectTypes = new List<Type>();

	public static void LoadAssembly([MarshalAs(UnmanagedType.LPUTF8Str)] string Path)
	{
		EngineLog.Print("Loading C# assembly...");
		LoadedAsm = Assembly.Load(File.ReadAllBytes(Path));

		LoadTypeFromAssembly("Log").GetMethod("LoadLogFunction").Invoke(null, new Delegate[] { EngineLog.Print });

		foreach (var i in LoadedAsm.GetTypes())
		{
			if (i.GetMethod("TickComponents") != null && !i.IsAbstract)
			{
				WorldObjectTypes.Add(i);
			}
		}

		StatsObject = LoadTypeFromAssembly("Stats");
	}

	public static Int32 Instantiate(string obj, EngineTransform t, IntPtr NativeObject)
	{
		foreach (var ObjectType in WorldObjectTypes)
		{
			if (ObjectType.Name == obj)
			{
				object? NewObject = Activator.CreateInstance(ObjectType);
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
				Set(ref NewObject, "NativeObject", NativeObject);

				WorldObjects.Add(HashCode, NewObject);
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
		if (!WorldObjects.ContainsKey(ID))
		{
			return;
		}

		ExecuteFunctionOnObject(ID, "DestroyObj");
		WorldObjects.Remove(ID);
	}

	public static System.Type? LoadTypeFromAssembly(string Type)
	{
		if (LoadedAsm == null)
		{
			EngineLog.Print("Tried to call method while the C# assembly is unloaded!");
			return null;
		}
		return LoadedAsm.GetType(Type);
	}


	public static void ExecuteFunctionOnObject(Int32 ID, [MarshalAs(UnmanagedType.LPUTF8Str)] string FunctionName)
	{
		if (!WorldObjects.ContainsKey(ID))
		{
			EngineLog.Print(string.Format("Tried to call {0} on the object with ID {1} but that object doesn't exist!", FunctionName, ID));
			return;
		}
		var obj = WorldObjects[ID];
		var func = obj.GetType().GetMethod(FunctionName);
		if (func == null)
		{
			EngineLog.Print(string.Format("Tried to call {0} on {1} but that function doesn't exist on this class!", FunctionName, obj.GetType().Name));
			return;
		}
		func.Invoke(obj, Array.Empty<object>());
	}

	public static object Get(object obj, string member)
	{
		var memb = obj.GetType().GetField(member);
		return memb.GetValue(obj);
	}
	public static void Set(ref object? obj, string field, object value)
	{
		var memb = obj.GetType().GetField(field);
		memb.SetValueDirect(__makeref(obj), value);
	}

	public static EngineVector GetVectorFieldOfObject(Int32 ID, [MarshalAs(UnmanagedType.LPUTF8Str)] string Field)
	{
		if (!WorldObjects.ContainsKey(ID))
		{
			EngineLog.Print(string.Format("Tried to access {1} of object with ID {0} but that object doesn't exist!", ID, Field));
			return new EngineVector();
		}
		var obj = WorldObjects[ID];
		var pos = Get(obj, Field);

		return new EngineVector((float)Get(pos, "X"), (float)Get(pos, "Y"), (float)Get(pos, "Z"));
	}

	public static void SetVectorFieldOfObject(Int32 ID, [MarshalAs(UnmanagedType.LPUTF8Str)] string Field, EngineVector NewValue)
	{
		if (!WorldObjects.ContainsKey(ID))
		{
			EngineLog.Print(string.Format("Tried to access {1} of object with ID {0} but that object doesn't exist!", ID, Field));
		}
		var obj = WorldObjects[ID];
		var pos = obj.GetType().GetField(Field).GetValue(obj);
		Set(ref pos, "X", NewValue.X);
		Set(ref pos, "Y", NewValue.Y);
		Set(ref pos, "Z", NewValue.Z);
		obj.GetType().GetField(Field).SetValue(obj, pos);
	}

	public static void SetDelta(float NewDelta)
	{
		StatsObject.GetField("DeltaTime").SetValue(null, NewDelta);
	}
}