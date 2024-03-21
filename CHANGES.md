# Changes

## Version 1.9.4

### Changes

- Added FXAA anti aliasing. (`aa_enabled` convar to enabled/disable)
- Removed "C#" toolbar option on projects that don't use C#.
- Added filtering options to the billboard component.

### Fixes

- Fixed issues with more than 8 lights at once.
- Fixed an issue with shadows in framebuffers that aren't the main one.
- Fixed an issue with boolean editor properties.
- Fixed an issue with occlusion culling with meshes that have a center position that isn't 0.

## Version 1.9.3

### Changes

- Networking stability improvements.

### Fixes

- Fixed a crash related to lists in object properties.
- Fixed a crash with the physics system in certain conditions.
- Fixed scaling for some shape types for collision visualization.
- Fixed glitchy behaviour of MoveComponent when colliding with multiple polygons at once.
- Fixed sound falloff being too abrupt.

## Version 1.9.2

### Changes

- Changed behavior of ResolutionScale.
- Added editor graphics settings.
- Added editor display settings.
- Text is now culled when not visible in a scroll area.
- Added a grid display in the editor.
- Added collision visualization.

### Fixes

- Improved how baked point light shadows are calculated.
- Fixed the MovementComponent sometiems letting the object to go through walls.
- Fixed a crash when a MeshComponent has no mesh loaded.
- Fixed a crash when reloading the C# assembly.

## Version 1.9.1

### Changes

- Added `bool PhysicsComponent::Attached`.
- Mesh object performance optimizations.
- Better sound falloff.

### Fixes

- Fixed some dialogs not appearing.

## Version 1.9.0

### Changes

- Added a new physics/collision system, using Jolt physics.
- Removed most of the old collision system.
- Completely rewrote MoveComponent.

### Fixes
- Fixed an issue where release builds on linux would fail to load any C# assembly.

## Version 1.8.2

### Fixes

- Fixed a crash with rebuilding C# in the editor.
- Fixed an issue with public EditorProperties.
- Fixed an issue where C# UICanvases would be garbage collected.
- Fixed some incomplete/incorrect documentation.

## Version 1.8.1

### Changes

- .tar.gz files for Linux builds.
- Added UIButton C# bindings.
- Added UICanvas C# function.
- Added C# `Engine.Stats.EngineConfig`.
- Added more documentation and updated README.md to mention the Linux version.

### Fixes

- Multiple small fixes to the editor.
- Linux builds can now properly create releases.

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
- Upgrade to new KlemmBuild version.
- Added `setup.sh` for Linux builds.

## Version 1.7.2

### Fixes

- Fixed a crash with the server.

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
- Fixed issues with shadows.
- Fixed an issue with the default movement.
