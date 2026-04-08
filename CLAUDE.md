# CLAUDE.md — Spacecraft Telemetry Visualizer

## Project Purpose

Portfolio C++ project demonstrating modern systems engineering: orbital mechanics, quaternion math, concurrency, and real-time 3D rendering. See `documentation/THE_PLAN.md` for the full 13-phase roadmap.

---

## Current State (April 2026)

**Phase 1 complete.** VTK has been fully removed and replaced with raw OpenGL 3.3 Core Profile via Qt's `QOpenGLWidget`. See `documentation/CHANGES_VTK_TO_OPENGL.md` for details.

**Working now:**
- Textured Earth sphere (Blue Marble JPEG, fallback solid blue)
- Satellite OBJ loaded via tinyobjloader, positioned at LEO (~1.06 Earth radii)
- ECI axes, sun indicator, orientation marker (XYZ triad)
- Blinn-Phong lit shader + unlit flat-color shader (GLSL 330)
- Orbit camera: left-drag rotates, scroll/right-drag zooms, middle-drag pans
- Roll/Pitch/Yaw sliders drive `SceneManager::updateOrientation()`

**Not yet built:** math library, orbital mechanics, telemetry engine, concurrency, simulation, tests, CI.

**Up next:** Phase 2 (module boundary enforcement) or Phase 3 (Vector3/Quaternion math library).

---

## Stack

| Component | Version / Notes |
|-----------|----------------|
| C++ standard | C++20 |
| Qt | 6.7.2 (`/home/andrzej/Qt/6.7.2/gcc_64`) |
| OpenGL | 3.3 Core Profile (system driver) |
| Build system | CMake 3.20+, Ninja |
| OBJ loading | tinyobjloader (bundled, `third_party/tinyobjloader/`) |
| VTK | **Removed** — do not reintroduce |

---

## Build & Run

```bash
# From project root
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
ninja -C build

# Run
./build/Spacecraft_Telemetry_Vizualizer
```

Assets (`models/`, `textures/`) are copied to `build/` automatically post-build.

---

## Module Structure

```
app/src/          — main.cpp only; sets OpenGL 3.3 Core surface format
gui/src/          — MainWindow (Qt UI, sliders, layout)
rendering/src/    — GLViewport, SceneManager, OrbitCamera, Geometry, ObjLoader
core/src/         — (placeholder) future: TelemetryEngine, OrbitalState, RingBuffer
math/src/         — (placeholder) future: Vector3, Quaternion
simulation/src/   — (placeholder) future: AttitudeSimulator
tests/            — (placeholder) future: GoogleTest or Catch2
third_party/      — tinyobjloader (MIT, header-only)
```

### Boundary Rules (enforce from Phase 2 onward)

- `core` — no Qt, no OpenGL, no rendering dependency
- `math` — no Qt, no UI, no rendering dependency
- `rendering` — must not call into `simulation`
- `gui` — may call `rendering` and `core`; must not call `simulation` directly

---

## Coding Conventions

- **No raw owning pointers.** Use `std::unique_ptr` for sole ownership, `std::shared_ptr` only when ownership is genuinely shared.
- **`const`-correct interfaces** throughout.
- **`constexpr`** wherever the compiler permits (critical in `math/`).
- **`explicit` constructors** on all single-argument constructors.
- **Value semantics** preferred; avoid dynamic allocation in math types.
- **C++20 features in use:** `std::jthread`, `std::stop_token` (planned for Phase 7).
- No global variables.
- Qt types (`QString`, `QMatrix4x4`, etc.) are confined to `gui/` and `rendering/`; they must not appear in `core/` or `math/`.

---

## Key Files

| File | Role |
|------|------|
| `rendering/src/GLViewport.h/cpp` | All OpenGL: shaders, VAO/VBO, camera, draw calls |
| `rendering/src/SceneManager.h/cpp` | Thin wrapper; public API used by MainWindow |
| `rendering/src/OrbitCamera.h/cpp` | Spherical-coordinate orbit camera |
| `rendering/src/ObjLoader.h/cpp` | tinyobjloader wrapper with vertex deduplication |
| `rendering/src/Geometry.h/cpp` | UV sphere generator (`MeshData` struct) |
| `gui/src/MainWindow.h/cpp` | Qt UI; connects sliders to SceneManager |
| `documentation/THE_PLAN.md` | Full roadmap — check here for phase goals and deliverables |
| `documentation/CHANGES_VTK_TO_OPENGL.md` | Migration record; explains what was removed/added |

---

## After Every Code Change

After completing any code change, always update documentation to match:

1. **`README.md`** — if the change affects build instructions, dependencies, prerequisites, or visible features.
2. **`documentation/THE_PLAN.md`** — mark tasks complete (add ✅), update the "Current State" section at the top, and note anything newly working or newly broken.

Do this as part of the same task, not as a follow-up. Do not ask whether to update — just do it.

---

## What to Avoid

- Do not add VTK back.
- Do not put Qt or OpenGL includes into `core/` or `math/`.
- Do not use `new`/`delete` for ownership; use smart pointers.
- Do not add features outside the current phase scope without asking.
- README.md is currently outdated (still mentions VTK) — don't rely on it for build instructions; use this file.
