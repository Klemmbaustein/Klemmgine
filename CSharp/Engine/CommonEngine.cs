using System.Runtime.InteropServices;
using System;
using Engine.Native;

/**
 * @defgroup CSharp C#
 * 
 * @brief
 * C# engine API.
 * 
 * Most C# classes have a C++ equivalent class/struct/namespace.
 * If there is an equivalent in C++, the differences between the C# version will be noted.
 */

/**
 * @ingroup CSharp
 * @brief
 * (C#) Engine namespace. Contains C# engine functions.
 * 
 * `Engine` is the namespace that contains all C# functions that belong to the engine.
 */
namespace Engine
{
	/**
	 * @brief
	 * Equivalent to Type::TypeEnum in C++.
	 * Some elements are missing.
	 */
	public enum NativeType
	{
		/// No type.
		Null = -1,
		/// Vector3.
		Vector3 = 0,
		/// float.
		Float = 1,
		/// Int32.
		Int = 2,
		/// string. Usually std::string.
		String = 3,
		/// Vector3. A UIVectorField will display 'R', 'G' 'B' instead of 'X', 'Y', 'Z' when editing a Vector3Color.
		Vector3Color = 4,
		/// Boolean type. Either true or false. Editor will display this as a checkbox.
		Bool = 7,
		/// Vector3. A UIVectorField will display 'P', 'Y' 'R' (Pitch, Yaw, Roll) instead of 'X', 'Y', 'Z' when editing a Vector3Color.
		Vector3Rotation = 8,
		/// List modifier. Bitwise or this with any other value in this enum to make it a list.
		List = 0b10000000
	};


	public static class CameraShake
	{
		private delegate void CameraShakeDelegate(float i);

		public static void PlayCameraShake(float Intensity)
		{
			NativeFunction.CallNativeFunction("PlayDefaultCameraShake", typeof(CameraShakeDelegate), new object[] { Intensity });
		}
	}

	/**
	 * @brief
	 * Contains functions related to the engine console.
	 * 
	 * Equivalent to Console namespace in <Engine/Console.h>. It needed to be
	 * renamed because it's original name conflicted with the System.Console namespace.
	 */
	public static class Command
	{
		internal delegate bool CommandDelegate([MarshalAs(UnmanagedType.LPUTF8Str)] string Command);

		/**
		 * @brief
		 * Executes a console command.
		 */
		public static bool Execute(string Command)
		{
			return (bool)NativeFunction.CallNativeFunction("CallConsoleCommand", typeof(CommandDelegate), new object[] { Command });
		}
	}

	/**
	 * @defgroup CSharp-Physics C# Physics
	 * @ingroup CSharp
	 * Functions/classes related to the physics system, C# scripting API.
	 */
	public static class Collision
	{
		private delegate HitResponse LineTraceDelegate(Vector3 Start,
			Vector3 End,
			[MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] IntPtr[] ComponentsToIgnore,
			int Length);

		public enum Layer : UInt16
		{
			/// No layer.
			None = 0b00000000,
			/// Collide with everything else on this layer.
			Layer0 = 0b00000001,
			/// Collide with everything else on this layer.
			Layer1 = 0b00000010,
			/// Collide with everything else on this layer.
			Layer2 = 0b00000100,
			/// Collide with everything else on this layer.
			Layer3 = 0b00001000,
			/// Collide with everything else on this layer.
			Layer4 = 0b00010000,
			/// @todo Implement or replace with another "Layer" collision type.
			Trigger = 0b00100000,
			/// Will collide with everything.
			Static = 0b01000000,
			/// Will only collide with static objects.
			Dynamic = 0b10000000

		}

		/**
		* @brief
		* Defines the motion type a PhysicsBody can have.
		* 
		* C++ equivalent: @ref Physics::MotionType
		* @ingroup CSharp-Physics
		*/
		public enum PhysicsMotionType
		{
			/// PhysicsBody cannot move.
			Static = 0,
			/// PhysicsBody is only movable using velocities only, does not respond to forces.
			Kinematic = 1,
			/// PhysicsBody is fully movable.
			Dynamic = 2,
		}

		/**
		 * @brief
		 * A struct containing information about a collision query.
		 */
		[StructLayout(LayoutKind.Sequential)]
		public struct HitResponse
		{
			/// True if the collision check hit something, false if not.
			[MarshalAs(UnmanagedType.U1)]
			public bool Hit;
			/// The point where the collision check hit something.
			public Vector3 ImpactPoint;
			/// The depth of the collision.
			public float Depth;
			/// The distance the collision ray has traveled, from 0 - 1. If the collision is not a RayCast or ShapeCast, this can be ignored.
			public float Distance;
			/// The object that was hit.
			private IntPtr HitObject;
			/// The component that was hit.
			private IntPtr HitComponent;
			/// The normal vector of the collision.
			public Vector3 Normal;

			/// Gets the object that was hit, in the form of a C# object class..
			public readonly WorldObject GetHitObject()
			{
				if (!Hit)
				{
					return null;
				}
				return WorldObject.GetObjectFromNativePointer(HitObject);
			}
		}

		/**
		 * @brief
		 * Casts a ray from Start to End.
		 * 
		 * @param Start
		 * The start position of the ray.
		 * @param End
		 * The end positionof the ray.
		 * @param ObjectsToIgnore
		 * Colliders that belong to these objects should be ignored.
		 */
		public static HitResponse RayCast(Vector3 Start, Vector3 End, WorldObject[] ObjectsToIgnore = null)
		{
			IntPtr[] ComponentPtrs;
			if (ObjectsToIgnore == null)
			{
				ComponentPtrs = [];
			}
			else
			{
				ComponentPtrs = new IntPtr[ObjectsToIgnore.Length];

				for (int i = 0; i < ObjectsToIgnore.Length; i++)
				{
					ComponentPtrs[i] = ObjectsToIgnore[i].NativePtr;
				}
			}

			return (HitResponse)NativeFunction.CallNativeFunction("NativeRaycast", typeof(LineTraceDelegate), [ Start, End, ComponentPtrs, ComponentPtrs.Length ]);
		}
	}

	public static class Stats
	{
		/// DeltaTime. Time between the last frame and the one before.
		public static float DeltaTime = 0.0f;

		public enum EngineConfig
		{
			/// Editor configuration. No UI or networking functions are active.
			Editor = 0,
			/// Debug configuration. Like release builds, but with a console and performance display.
			Debug = 1,
			/// Release configuration. Release build of the game.
			Release = 2,
			/// Server configuration. For game servers. No graphics or UI code is active.
			Server = 3
		}

		/// The configuration of the game engine.
		public static EngineConfig Config = EngineConfig.Editor;
	}


	/**
	 * @brief
	 * Class containing functions related to scenes.
	 * 
	 * C++ equivalent: Scene namespace.
	 * 
	 * @ingroup CSharp
	 */
	public static class Scene
	{
		private delegate void LoadSceneDelegate([MarshalAs(UnmanagedType.LPUTF8Str)] string SceneName);
		/**
		 * @brief
		 * Loads a new scene with from the given name.
		 * 
		 * C++ equivalent: Scene::LoadNewScene
		 * 
		 * @param SceneName
		 * The name of the scene file, without a path or extension.
		 */
		public static void LoadScene(string SceneName)
		{
			NativeFunction.CallNativeFunction("LoadScene", typeof(LoadSceneDelegate), new object[] { SceneName });
		}
	}

	public static class Sound
	{
		private delegate IntPtr LoadSoundDelegate([MarshalAs(UnmanagedType.LPUTF8Str)] string SoundName);
		private delegate void UnloadSoundDelegate(IntPtr SoundBuffer);
		private delegate void PlaySound(IntPtr s, float Pitch, float Volume, bool Looping);

		public class SoundBuffer
		{
			readonly IntPtr BufferPtr = new();
			public SoundBuffer(IntPtr Buffer)
			{
				BufferPtr = Buffer;
			}

			public void Play(float Pitch = 1, float Volume = 1, bool Looping = false)
			{
				NativeFunction.CallNativeFunction("PlaySound", typeof(PlaySound), new object[] { BufferPtr, Pitch, Volume, Looping });
			}
			~SoundBuffer()
			{
				NativeFunction.CallNativeFunction("UnloadSound", typeof(UnloadSoundDelegate), new object[] { BufferPtr });
			}
		}

		public static SoundBuffer LoadSound(string File)
		{
			IntPtr BufferPtr = (IntPtr)NativeFunction.CallNativeFunction("LoadSound", typeof(LoadSoundDelegate), new object[] { File });
			return new SoundBuffer(BufferPtr);
		}
	}
}