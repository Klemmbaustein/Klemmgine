# Changelog

## Version 1.6.2

### C#
- Upgraded .net version to .net 8.0
- The .net runtime is no longer packaged with the engine but pulled from the installed .net sdk when creating a standalone build.
- Added `-netVersion` argument to ProjectGenerator (default value `.net8.0`)
- Added `Project.GetProjectName()` function.

### Fixes
- Fixed `Project.OnLaunch()` never being called.
- Fixed multiple issues related to light mode.
- When using pre-built binaries, you no longer have the uneccesary option to disable C#.