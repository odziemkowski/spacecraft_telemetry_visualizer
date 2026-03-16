# Spacecraft Telemetry Visualizer — Engineering Roadmap

---

## Current State

- **Stack:** C++20, Qt 6.10, VTK 9.5.2, CMake + Ninja, Linux
- **Working:** Qt + VTK rendering pipeline integrated, `QVTKOpenGLNativeWidget` embedded in `QMainWindow`
- **Working:** Satellite OBJ model loaded and displayed in 3D scene, orientation marker (axes triad) in viewport corner
- **Working:** Roll / Pitch / Yaw sliders drive real-time orientation updates
- **Module layout defined:** `app/`, `gui/`, `rendering/`, `core/`, `math/`, `simulation/`, `tests/` — placeholder directories present
- **Not yet built:** Math library, orbital mechanics, telemetry engine, concurrency, simulation, testing, CI

---

## Phase 1 — MVP: 3D Viewer with Manual Controls ✅

**Goal:** Prove the Qt + VTK stack works and produce something visually meaningful.

**Tasks:**
- Set up CMake project with Qt6 and VTK
- Embed `QVTKOpenGLNativeWidget` inside `QMainWindow`
- Load satellite OBJ model; fallback to sphere if missing
- Add orientation marker (XYZ triad) in viewport corner
- Wire Roll / Pitch / Yaw sliders to `SceneManager::updateOrientation()`

**Deliverables:**
- Satellite visible and rotatable in 3D
- Sliders drive orientation in real time
- Clean build, project on GitHub

---

## Phase 2 — Architecture: Clean Module Boundaries

**Goal:** Refactor the MVP into a modular structure with enforced separation of concerns and modern C++ ownership semantics throughout.

**Tasks:**
- Establish hard module boundaries: `core` has no Qt/VTK dependency, `math` has no UI dependency, `rendering` does not call into `simulation`
- Replace remaining raw pointers with `std::unique_ptr` / `std::shared_ptr` where ownership is shared
- Audit `const` correctness across all existing interfaces
- Split `CMakeLists.txt` into per-module targets linked together at the top level
- Update `README.md` with an architecture diagram showing module dependency graph

**Deliverables:**
- Each module compiles independently as a CMake target
- No cross-layer includes violating the dependency rules
- Architecture diagram in `README.md`

**Stretch:**
- Add a `CONTRIBUTING.md` that codifies the boundary rules

---

## Phase 3 — Math Module: Vector3 and Quaternion

**Goal:** Build a self-contained, well-tested math library that underpins all later work. No external math dependencies.

**Tasks:**
- Implement `Vector3`: addition, subtraction, scalar multiply, dot product, cross product, norm, normalize
- Implement `Quaternion`: construction, normalization, multiplication, conjugate, rotate-vector, to-rotation-matrix
- Apply `constexpr` throughout where the compiler permits
- Use `explicit` constructors; value semantics only; zero dynamic allocation
- Write unit tests for all operations including edge cases (zero vector, identity quaternion, gimbal-adjacent inputs)

**Deliverables:**
- `math/src/Vector3.h`, `math/src/Quaternion.h` — header-only or minimal `.cpp`
- Test suite passes: all vector and quaternion operations verified
- `math` module has zero dependency on Qt, VTK, or any other module

**Stretch:**
- Implement SLERP (spherical linear interpolation) — required for telemetry interpolation in Phase 6

---

## Phase 4 — Solar System Scene: Earth and ECI Frame

**Goal:** Replace the featureless void with a Solar System context. A satellite in orbit around a textured Earth is immediately more legible as a real system than a model spinning in empty space.

**Tasks:**
- Add a textured Earth sphere using a NASA Blue Marble or equivalent public-domain texture via VTK texture mapping
- Add a simplified Sun representation (distant directional light or emissive sphere) to establish the illumination axis
- Set scene scale consistent with real orbital altitudes (LEO ~400 km above Earth radius 6371 km) — normalized but proportionally correct
- Define an Earth-Centered Inertial (ECI) coordinate frame in the scene: X toward vernal equinox, Z toward north pole, axes visualized
- Place the satellite actor at a static orbital position above Earth as a placeholder

**Deliverables:**
- Earth sphere rendered with texture in the scene
- Satellite positioned at a fixed orbital altitude above Earth surface
- ECI frame axes visible in the scene
- Scene scale documented in README

**Stretch:**
- Add a starfield background (VTK skybox or point cloud)
- Add atmospheric scattering approximation (gradient halo around Earth limb)
- Camera presets: zoom to Earth, zoom to satellite

---

## Phase 5 — Orbital Mechanics: SGP4 Propagation and Real Satellites

**Goal:** Pull real satellite orbital data from Celestrak (TLE format) and propagate positions using the industry-standard SGP4 model. The satellite now moves in a physically correct orbit around the Earth scene.

**Tasks:**
- Integrate `libsgp4` (Vallado's C++ implementation, MIT-licensed) via CMake `FetchContent` or system package
- Implement a TLE parser: load a two-line element set, extract epoch and Keplerian elements
- Wire SGP4 propagation to a time input: given a `std::chrono::system_clock::time_point`, compute ECI position and velocity
- Define `struct OrbitalState { TimePoint epoch; Vector3 position_eci; Vector3 velocity_eci; }` in `core/`
- Position the satellite actor in the scene using the propagated ECI coordinates mapped to scene scale
- Test with at least one real satellite (e.g., ISS NORAD ID 25544) fetched from Celestrak

**Deliverables:**
- Satellite tracks a real TLE-derived orbit around the Earth scene
- `OrbitalState` defined in `core/` and consumed by both rendering and simulation layers
- Propagation verified against known reference positions

**Stretch:**
- Fetch TLE data live from Celestrak API at startup
- Visualize the full orbital path as a 3D ellipse in the scene
- Render a 2D ground track projected onto a map panel

---

## Phase 6 — Telemetry Engine: Attitude Data with Interpolation

**Goal:** Layer attitude telemetry on top of orbital position. The satellite now has both a correct orbital position (from SGP4) and an attitude (from telemetry), driving a complete state representation.

**Tasks:**
- Define `struct SatelliteState { TimePoint timestamp; Vector3 position_eci; Quaternion attitude; }` in `core/`
- Implement `TelemetryEngine`:
  - `bool load_from_csv(const std::string& path)` — parse timestamped attitude records
  - `std::optional<SatelliteState> state_at(TimePoint t) const` — interpolated query
- Store records ordered by timestamp; use binary search for O(log n) lookup
- Implement SLERP interpolation for quaternion attitude between samples (from Phase 3 math module)
- Use `std::chrono` for all time representation; `std::optional` for safe miss-handling
- Write unit tests: interpolation at boundary, beyond-range query, malformed CSV

**Deliverables:**
- `TelemetryEngine` class in `core/src/`
- CSV-driven attitude playback overlaid on orbital position
- Unit tests covering interpolation edge cases

**Stretch:**
- Support JSON telemetry format in addition to CSV
- Flag samples with quaternion norm deviation > epsilon

---

## Phase 7 — Concurrency: Background Pipeline

**Goal:** Move telemetry production and time advancement off the UI thread. The UI remains responsive at all times; data flows through a producer-consumer pipeline.

**Tasks:**
- Create a dedicated telemetry/simulation thread using `std::jthread` (C++20) with cooperative cancellation via `std::stop_token`
- Implement a thread-safe state queue: producer posts `SatelliteState`, consumer (UI thread) reads latest
- Use `std::mutex` + `std::condition_variable` for synchronization; `std::lock_guard` / `std::unique_lock` — no manual unlock
- Ensure clean shutdown: thread exits before any owned resources are destroyed (RAII lifecycle)
- No global variables; shared ownership documented explicitly

**Deliverables:**
- Telemetry thread runs independently of the render loop
- UI thread never blocks on telemetry computation
- No data races (verify with `-fsanitize=thread`)
- README: threading model diagram

**Stretch:**
- Implement a lock-free single-producer / single-consumer ring buffer as a learning exercise and benchmarking comparison

---

## Phase 8 — Simulation Engine: Attitude Dynamics

**Goal:** Add a physics-based simulation mode that propagates attitude using angular velocity integration. The application can run in two modes: replay recorded telemetry, or simulate attitude dynamics in real time.

**Tasks:**
- Implement quaternion attitude propagation: `q_dot = 0.5 * q ⊗ [0, ω]`, integrated with a fixed timestep
- Normalize quaternion periodically to prevent drift
- Expose a `SimulatedMode` vs `TelemetryMode` toggle in the UI
- In simulated mode, angular velocity is configurable via UI controls
- Unit tests for propagation: known angular velocity → verify quaternion after N steps matches analytical result

**Deliverables:**
- `AttitudeSimulator` class in `simulation/src/`
- Both modes selectable and working at runtime
- Propagation unit tests pass

**Stretch:**
- Inject configurable Gaussian noise to simulate sensor drift
- Show error comparison overlay: simulated vs telemetry attitude deviation displayed as an angle

---

## Phase 9 — Time Controls: Playback and Ring Buffer

**Goal:** Give the user full control over time. Playback speed, pause, step, and real-time live simulation all operate correctly.

**Tasks:**
- Implement time scaling: 0.1x, 0.5x, 1x, 2x, 10x playback speed
- Implement pause and resume without state loss
- Implement single-frame step (forward and backward)
- Add a ring buffer for live simulation output: fixed-capacity circular storage with overwrite-oldest policy
- Display a timeline scrubber or elapsed-time indicator in the UI

**Deliverables:**
- All time controls functional in both telemetry and simulated modes
- Ring buffer implementation in `core/src/RingBuffer.h` (template, header-only)
- Smooth playback across speed changes without visual stuttering

**Stretch:**
- Benchmark ring buffer throughput vs `std::deque` baseline and document results

---

## Phase 10 — Testing and CI

**Goal:** Establish automated testing as a first-class project artifact. Every push is verified.

**Tasks:**
- Integrate GoogleTest or Catch2 via CMake `FetchContent`
- Write or migrate tests for: quaternion math, vector math, interpolation boundaries, telemetry parsing errors, ring buffer wrap-around, thread safety scenarios
- Set up GitHub Actions workflow: build on ubuntu-latest, run test suite, report results
- Enforce that `cmake --build` + `ctest` passes before merging to `main`

**Deliverables:**
- `tests/` populated with structured test files per module
- CI badge in `README.md`
- All tests green on push

**Stretch:**
- Add code coverage reporting (gcov + lcov) to the CI pipeline

---

## Phase 11 — Logging and Configuration

**Goal:** Make the application behave like real software: structured logs, externally configurable behavior, no hardcoded paths.

**Tasks:**
- Integrate `spdlog` for structured logging: info, debug, warn, error levels; file sink + console sink
- Define a JSON configuration file (`config.json`) for: model path, initial TLE source, playback speed, simulation parameters
- Parse config at startup using `nlohmann/json` (header-only, via `FetchContent`)
- Remove all hardcoded paths from source; derive everything from config or application directory

**Deliverables:**
- Application fully config-driven at startup
- Structured logs written to file per session
- `config.json` with documented fields committed to repo

**Stretch:**
- Live config reload: watch `config.json` for changes and apply without restart

---

## Phase 12 — Performance and Memory Discipline

**Goal:** Demonstrate systems-level thinking about data layout, copy elision, and profiling.

**Tasks:**
- Audit all hot-path data structures for unnecessary copies; apply move semantics where copies are avoidable
- Review telemetry storage layout: evaluate struct-of-arrays vs array-of-structs for interpolation access pattern
- Profile the render loop with `std::chrono` timing points; identify and address frame-time spikes
- Review all `std::shared_ptr` usage; replace with `std::unique_ptr` where ownership is not genuinely shared

**Deliverables:**
- README section documenting profiling methodology, findings, and changes made
- No regressions in test suite after optimization passes

**Stretch:**
- Implement struct-of-arrays layout for telemetry storage and benchmark against baseline
- Add a frame-time histogram display in debug mode

---

## Phase 13 — Polish and Portfolio Positioning

**Goal:** Turn a well-engineered codebase into a standout portfolio artifact.

**Tasks:**
- Rewrite `README.md` with: architecture diagram, module dependency graph, threading model diagram, math design notes, design tradeoffs section
- Record a short demo video (screen capture) showing: Earth scene, satellite in orbit, telemetry playback, simulated mode, time controls
- Add a feature list and "what this demonstrates" section connecting implementation choices to C++ engineering competencies
- Audit commit history for clarity; ensure commit messages describe intent not just mechanism

**Deliverables:**
- `README.md` suitable for a hiring manager and a senior C++ engineer simultaneously
- Demo video linked from README
- Project presentable without a walkthrough

**Stretch:**
- Publish a technical write-up explaining the quaternion math and threading design
- Add ECI to ECEF frame conversion and visualize the ground track on a projected globe texture

---

## Portfolio Signal

| Competency | Evidence |
|---|---|
| Modern C++ (C++20) | `std::jthread`, `std::stop_token`, `constexpr`, move semantics, RAII throughout |
| Mathematical reasoning | Custom quaternion/vector library, SLERP, attitude propagation |
| Systems architecture | Enforced module boundaries, dependency inversion, no circular includes |
| Concurrency | Producer-consumer pipeline, thread-safe queue, sanitizer-clean |
| Data pipeline design | TLE ingestion → SGP4 → ECI position → telemetry overlay → render |
| Orbital mechanics | SGP4 propagation, ECI frame, real satellite tracking from Celestrak |
| Testing discipline | Unit tests per module, CI on every push, edge case coverage |
| Performance awareness | Profiling, layout analysis, move semantics audit, documented tradeoffs |

> A serious C++ engineer capable of building technical systems in a domain where correctness is not optional.
