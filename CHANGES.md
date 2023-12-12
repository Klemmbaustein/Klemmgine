# Changelog

## Version 1.6.1

- Exposed more Functions to C#:
	- `CollisionComponent::OverlapCheck`
	- `UIBackground` class
	- Added `Log.Print.Severity` (Info, Warning, Error)
- Fixed an issue that would cause C# projects to fail to build.
- Fixed an íssue where `Project.OnLaunch()` Would never be called.