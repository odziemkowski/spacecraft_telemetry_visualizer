# VTK to Raw OpenGL Migration

## Summary

Replaced the VTK rendering backend with raw OpenGL 3.3 Core Profile via Qt's `QOpenGLWidget`. The VTK dependency has been fully removed. All visual functionality is preserved: textured Earth, lit satellite OBJ model, ECI axes, sun indicator, orientation marker, and interactive camera.

## What Changed

### New Files

| File | Purpose |
|------|---------|
| `rendering/src/GLViewport.h/cpp` | QOpenGLWidget subclass — all OpenGL rendering, shaders, VAO/VBO management, mouse camera interaction |
| `rendering/src/Geometry.h/cpp` | `MeshData` struct and procedural UV sphere generator |
| `rendering/src/OrbitCamera.h/cpp` | Orbit camera (spherical coordinates with mouse rotate/zoom/pan) |
| `rendering/src/ObjLoader.h/cpp` | OBJ model loading via tinyobjloader with vertex deduplication |
| `third_party/tinyobjloader/tiny_obj_loader.h` | Single-header OBJ loader library (MIT license) |

### Modified Files

| File | Change |
|------|--------|
| `rendering/src/SceneManager.h` | Removed all VTK includes and members. Now holds a `GLViewport*` pointer |
| `rendering/src/SceneManager.cpp` | Reduced from ~280 lines of VTK code to ~30 lines delegating to GLViewport |
| `CMakeLists.txt` | Removed `VTK_DIR`, `find_package(VTK ...)`, `${VTK_LIBRARIES}`. Added new source files and tinyobjloader include path |
| `app/src/main.cpp` | Replaced VTK surface format setup with OpenGL 3.3 Core Profile request |

### Unchanged Files

| File | Note |
|------|------|
| `gui/src/MainWindow.h/cpp` | No changes — SceneManager's public interface (`setup()`, `updateOrientation()`, `widget()`) is identical |

## Architecture

```
MainWindow
  └── SceneManager (thin wrapper, same public API)
        └── GLViewport (QOpenGLWidget + QOpenGLFunctions_3_3_Core)
              ├── Lit shader (Blinn-Phong: Earth sphere, satellite OBJ)
              ├── Unlit shader (flat color: ECI axes, sun indicator, orientation marker)
              ├── OrbitCamera (spherical coords, mouse-driven)
              ├── Geometry (UV sphere generator)
              └── ObjLoader (tinyobjloader wrapper)
```

## Rendering Details

- **Shaders:** Two GLSL 330 programs — lit (Blinn-Phong with directional + ambient light, optional texture) and unlit (flat color)
- **Earth:** 64x64 UV sphere, Blue Marble JPEG texture loaded via QImage, fallback to solid blue
- **Satellite:** OBJ loaded via tinyobjloader, scaled to 4% of Earth diameter, positioned at LEO (1.0628 Earth radii). Fallback sphere if OBJ missing
- **Lighting:** Directional sun light from (1.0, 0.3, 0.2) matching VTK scene, dim ambient fill from opposite direction
- **Camera:** Orbit camera — left-drag rotates, scroll/right-drag zooms, middle-drag pans. Clipping range 0.001–100.0
- **Orientation marker:** XYZ axes triad in bottom-left 20% corner via glViewport scissoring, rotation-only view matrix
- **Rotation order:** Matches VTK's SetOrientation(rx,ry,rz): X then Y then Z

## Build

VTK is no longer required. Dependencies are now:
- Qt 6 (Core, Widgets, OpenGLWidgets)
- OpenGL 3.3 (provided by system driver)
- tinyobjloader (bundled, header-only)
