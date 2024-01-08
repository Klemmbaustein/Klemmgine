# Changes

## Version 1.8.0

### Changes

- Large editor UI rewrite.
- Added documentation using doxygen.
- Moved engine functions into `KlemmgineCSharp` project.
- Renamed some C# functions to match their C++ equivalent.
- Added enum `Input::GamepadType` and `Input::GetGamepadType(Input::Gamepad* From)`.
- `UIButton` and `UITextField` now inherit from `UIBackground`.
- Added `Engine.NativeObject` and a few more utility functions to C#.
- Added C# `EditorProperty` attribute and functionality.

## Version 1.7.2

### Fixes

- Fixed a crash with the server

## Version 1.7.1

### Changes
- When adding a C# class, the path will now default to the path opened in the class browser.

### Fixes
- Fixed the `Mem` display in the editor being broken on Linux.
- Fixed server not launching on Linux when pressing `Run` in the editor.


## Version 1.7.0

### Changes
- Added controller input support. (Engine/Gamepad.h)
	- Added C# bindings for controller input.
- Added Linux support.
- Added pre-built binaries for Linux (maybe?)

### Fixes
- Fixed some issues with point lights.
- Fixed isues with shadows.
- Fixed an issue with the default movement.