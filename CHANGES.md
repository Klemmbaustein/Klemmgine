# Changelog

## Version 1.6.1

- Exposed more Functions to C#:
	- `CollisionComponent::OverlapCheck`
	- `UIBackground` class.
	- `UIText` class.
	- Added `Log.Print.Severity` enum (Info, Warning, Error).
- Changed some static C# classes to namespaces to avoid name conflicts.
- Fixed an issue that would cause C# projects to fail to build.
- Fixed an íssue where `Project.OnLaunch()` Would never be called.
- Fixed an issue with the `Collision.HitResponse` C# struct.
- Fixed an issue with ParticleComponents ignoring position in C#.
- Fixed an issue that would cause C++ projects to fail to launch if		
  [ProjectName]-Server.exe didn't exist and 'Launch with Server' was disabled.
- Fixed multiple issues with the fallback material.