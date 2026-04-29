# Astrodynamics Engine — Engineering Roadmap

**70-100 hours | 3-5 hrs/week | ~5-6 months**

---

## Design Principles

Every phase produces a demonstrable, visually compelling result. If the project stops at any phase, the portfolio still holds up. The visual foundation comes first — real-world data, real missions, something you'd actually want to watch. The deep engineering (custom propagators, Monte Carlo) is built on top of that foundation, not instead of it.

**Rendering Stack: Custom OpenGL, not VTK.**
The project uses raw OpenGL 3.3 Core Profile via Qt's `QOpenGLWidget`. This is deliberate — it demonstrates low-level graphics knowledge and gives full control over the rendering pipeline. All shaders are hand-written. No VTK, no engine wrappers.

---

## Current State

- **Stack:** C++20, Qt 6.10, OpenGL 3.3 Core, CMake + Ninja, Linux
- **Working:** Custom OpenGL rendering pipeline with lit (Blinn-Phong) and unlit shader programs
- **Working:** Textured Earth sphere (Blue Marble 2K), satellite OBJ model at orbital altitude
- **Working:** ECI coordinate axes, sun indicator, orientation marker (corner triad)
- **Working:** Orbit camera (mouse-driven rotate/zoom/pan), Roll/Pitch/Yaw sliders
- **Module layout:** `app/`, `gui/`, `rendering/`, `core/`, `math/`, `simulation/`, `tests/`
- **Not yet built:** Moon, Sun positioning, time engine, ephemeris, mission trajectories, propagation, analytics

---

## Timeline Overview

| Phase | Focus | Hours | Weeks |
|-------|-------|-------|-------|
| 0 | 3D Viewer with Manual Controls | -- | -- |
| 1 | Solar System: Earth, Moon, Sun + Rendering Infrastructure | 10-14 | 3-4 |
| 2 | Time Engine and Orbit Trails | 8-10 | 2-3 |
| 3 | Mission Visualization: Artemis II | 10-14 | 3-4 |
| 4 | Analytical Dashboard | 8-12 | 2-3 |
| 5 | Eigen + RK4 + Two-Body Propagation | 8-10 | 2-3 |
| 6 | Perturbation Physics: J2 + Drag | 8-10 | 2-3 |
| 7 | Monte Carlo Concurrency Engine | 10-15 | 3-4 |
| 8 | Testing, CI, Portfolio Polish | 6-8 | 2 |
| | **Total** | **70-97** | **19-27** |

**Part I (Phases 0-3):** Visual foundation — a solar system visualizer with real data and a real mission. This is the portfolio hook.
**Part II (Phases 4-7):** Engineering depth — analytical tools, custom propagation, perturbation physics, Monte Carlo concurrency. This is the portfolio substance.
**Part III (Phase 8):** Polish — testing, CI, documentation. Can overlap with any phase.

---

## Part I — Visual Foundation

---

### Phase 0 — MVP: 3D Viewer with Manual Controls [DONE]

**Goal:** Prove the Qt + OpenGL stack works and produce something visually meaningful.

**Delivered:**
- Custom OpenGL 3.3 Core renderer with Blinn-Phong lighting
- Textured Earth sphere (Blue Marble), satellite OBJ model
- ECI axes, sun indicator, orientation marker
- Orbit camera with mouse interaction
- Roll/Pitch/Yaw sliders driving real-time orientation

---

### Phase 1 — Solar System: Earth, Moon, and Sun

**10-14 hours | ~3-4 weeks**

**Goal:** Turn the scene from "a sphere with a satellite" into a recognizable piece of the solar system. Earth, Moon, and Sun at correct relative positions, with the Moon visibly orbiting Earth. More importantly, upgrade the rendering pipeline to handle the vast distance scales that space demands — this is foundational infrastructure for everything that follows.

**Why This Next:**
A textured Earth with a Moon at the correct distance immediately elevates the project from a rendering demo to something that looks like aerospace software. But to render it *correctly* — a spacecraft at 10m detail orbiting a planet 6,371 km wide with a moon 384,400 km away — requires solving the depth precision and coordinate precision problems that every serious space visualizer faces. Getting this right is a deep technical signal.

#### Rendering Infrastructure: Handling Space-Scale Distances

The standard OpenGL depth buffer is 24-bit linear — it gives usable precision across maybe 3-4 orders of magnitude. The Earth-Moon system spans ~8 orders (meters to hundreds of thousands of km). Three techniques, in priority order:

**1. Logarithmic Depth Buffer** — *the single most impactful technique*
- Replace the standard linear depth mapping with logarithmic: `gl_FragDepth = log(z/near) / log(far/near)`
- A 24-bit log depth buffer covers near=0.1m to far=1e9m with no z-fighting
- Implementation: modify both vertex and fragment shaders. The vertex shader writes a logarithmic clip-space z; the fragment shader writes `gl_FragDepth` to correct for rasterizer interpolation
- This alone handles the Earth-Moon system comfortably

**2. Camera-Relative Rendering (Floating Origin)** — *precision fix*
- When the camera is near the Moon (~384,400 km from Earth origin), storing positions as floats relative to Earth center loses sub-meter precision
- Fix: subtract camera position in **double precision on CPU**, pass the resulting small offsets as float to GPU
- All model/view matrix computation happens in doubles; only the final camera-relative matrix goes to the shader as float
- This is how Celestia, SpaceEngine, and KSP handle the problem

**3. Multi-Layer Compositing** — *stretch / future*
- Render near objects (spacecraft, nearby surface) and far objects (distant bodies, starfield) in separate passes with independent near/far clip planes, composite via framebuffer
- Not needed if log depth + floating origin handle the range, but available if we later want sub-meter surface detail alongside solar-system-scale views
- Adds complexity: multiple render passes, FBO management, depth-aware compositing

**Scale approach:**
- Use real physical units internally (kilometers), not arbitrary scene units
- Earth radius = 6,371 km, Moon distance = ~384,400 km, Moon radius = 1,737 km
- Sun as directional light from computed ecliptic position (not geometry at 150M km). A small emissive indicator on the skybox edge shows the Sun direction
- Log depth buffer near/far: 0.001 km (1 meter) to 1e6 km — covers Earth-Moon system with room to spare

**Ephemeris approach:**
- Analytical approximations (Jean Meeus, *Astronomical Algorithms*) for Moon and Sun geocentric positions
- Accurate to arcminutes — more than sufficient for visualization
- Self-contained: zero external dependencies, no network calls, no SPICE kernels
- Implement in `math/` or `core/` as pure functions: `moon_position_eci(JulianDate) -> Vector3d`, `sun_direction_eci(JulianDate) -> Vector3d`

**Tasks:**
- **Rendering infrastructure:**
  - ✅ Implement logarithmic depth buffer in vertex + fragment shaders
  - Refactor matrix pipeline to use double-precision computation on CPU, camera-relative float output to GPU
  - Verify: no z-fighting across the full Earth-to-Moon distance range
- **Scene content:**
  - Implement Julian Date conversion from calendar date/time (`std::chrono` -> JD)
  - Implement analytical Moon position (geocentric ecliptic -> ECI): longitude, latitude, distance from Meeus Ch. 47
  - Implement analytical Sun direction (geocentric ecliptic -> ECI): Meeus Ch. 25
  - Add textured Moon sphere (public-domain lunar texture, ~2K resolution)
  - Position Moon using computed ECI coordinates in real km
  - Update Sun directional light to use computed Sun direction instead of hardcoded vector
  - Add a starfield background (random point cloud on a far sphere, or skybox)

**Deliverables:**
- Log depth buffer + camera-relative rendering working — no z-fighting from Earth surface to Moon distance
- Moon visible at correct orbital distance from Earth, textured
- Sun light direction matches real solar position for a given date
- Starfield background replacing the flat dark color
- Camera can smoothly zoom from spacecraft-scale detail to full Earth-Moon system view

**Stretch:**
- Multi-layer compositing for extreme near/far separation (sub-meter near + solar system far)
- Moon phase visually correct (lit side faces Sun direction)
- Earth axial tilt (23.4 deg) applied relative to ecliptic
- Render Moon's orbit path as a faint ellipse

> **Claude's note:** The log depth buffer is genuinely one of the most impressive low-level OpenGL features to have in a portfolio — it shows you understand the depth pipeline beyond `glEnable(GL_DEPTH_TEST)`. The camera-relative rendering is equally important and often overlooked: it's a precision problem, not a rendering problem, and explaining the float32 precision limit at large world coordinates is a great interview topic. Together, these two techniques are what separate a toy renderer from one that can handle real-world scale.
>
> For the Meeus ephemeris: the algorithms are the sweet spot — analytical, well-documented, public domain, battle-tested. VSOP87 and ELP2000 are more precise but vastly more complex to implement. For a visualizer (not a navigation system), Meeus is the right call. The Julian Date utility will be reused everywhere — worth getting right and testing thoroughly.

---

### Phase 2 — Time Engine and Orbit Trails

**8-10 hours | ~2-3 weeks**

**Goal:** Make the solar system move. A simulation clock with variable speed lets the user watch the Moon orbit Earth, see the Sun's apparent motion shift, and experience orbital mechanics as something dynamic and watchable.

**Why This Next:**
Static positions are a screenshot. A time engine turns the project into a simulation. Speeding up time so the Moon visibly orbits Earth is the moment this stops being a rendering exercise and starts being a tool. It's also the infrastructure that every later phase depends on — mission playback, propagation, analytics all need a controllable clock.

**Tasks:**
- Implement `SimulationClock` in `core/`:
  - Internal time as Julian Date (double precision)
  - `set_epoch(JulianDate)` — jump to a specific moment
  - `advance(dt_real_seconds, time_scale)` — tick forward by real elapsed time * scale factor
  - `now() -> JulianDate` — current simulation time
  - Time scales: 1x, 10x, 100x, 1000x, 10000x (at 10000x, one Moon orbit takes ~2.7 seconds — satisfying to watch)
- Implement time control UI: play/pause button, speed selector, epoch date/time picker, timeline scrubber
- Connect `SimulationClock` to the render loop: each frame, advance clock by frame delta * scale, recompute Moon/Sun positions, update scene
- Implement orbit trail rendering:
  - Store last N positions as a GL_LINE_STRIP
  - Fade opacity from head (bright) to tail (transparent) — shows direction of motion
  - Configurable trail length (e.g., 1 orbit period worth of points)
- Add smooth camera tracking modes: lock-on-Earth, lock-on-Moon, free camera

**Deliverables:**
- Moon visibly orbits Earth when time is accelerated
- Sun direction shifts over the year
- Orbit trails show the Moon's path as a fading line
- Time controls: play/pause, speed adjustment, epoch selection
- Smooth, responsive — no jitter or stutter at high time scales

**Stretch:**
- Frame-rate-independent time stepping (decouple physics tick from render tick)
- Keyboard shortcuts for time controls (space = pause, +/- = speed)
- Display current simulation date/time as HUD overlay

> **Claude's note:** The trail rendering is where the OpenGL investment pays off — VTK would make fading trails painful. With raw GL, it's a vertex buffer with per-vertex alpha, trivial. For time scales above ~1000x, consider decoupling the physics step from the render frame: run multiple clock ticks per frame rather than one big jump, so the trail points are evenly spaced. This also prepares the architecture for Phase 7's background propagation.

---

### Phase 3 — Mission Visualization: Artemis II

**10-14 hours | ~3-4 weeks**

**Goal:** Load and play back the trajectory of a real space mission. Artemis II — NASA's crewed lunar flyby — traces a free-return path around the Moon. Watching the spacecraft accelerate away from Earth, swing around the Moon, and return is visually dramatic and immediately demonstrates that the application works with real-world data.

**Why Artemis II:**
- **Current and relevant** — an active NASA program, great conversation starter
- **Visually dramatic** — the free-return trajectory is a beautiful curve that swings close to the lunar surface
- **Data is public** — NASA publishes trajectory data via SPICE kernels and JPL Horizons
- **Right complexity** — one spacecraft, Earth-Moon system, ~10 day mission duration. Complex enough to be impressive, contained enough to be achievable

**Data approach:**
- Pre-compute trajectory from NASA SPICE kernels or JPL Horizons API, export as CSV/binary: `[timestamp, x, y, z, vx, vy, vz]` in ECI frame
- Ship the data file with the project (small — a few thousand data points for a 10-day mission)
- Interpolate between data points using cubic Hermite spline (position + velocity at each knot gives C1 continuity)
- No runtime dependency on SPICE or network access

**Tasks:**
- Obtain Artemis II trajectory data and convert to a project-local format (CSV or compact binary)
- Implement trajectory data loader in `core/`: parse file, store as time-ordered state vectors
- Implement interpolation: given a `JulianDate`, return interpolated position (and optionally velocity)
- Render the spacecraft along the trajectory, driven by `SimulationClock`
- Render the full trajectory path as a 3D curve (color-coded by velocity magnitude or mission phase)
- Add mission event markers: TLI (trans-lunar injection), lunar closest approach, Earth return
- Implement camera modes: follow-spacecraft (camera tracks the vehicle), Earth-centered, Moon-centered
- Display mission telemetry HUD: altitude above Earth/Moon, velocity magnitude, mission elapsed time

**Deliverables:**
- Artemis II trajectory plays back in the Earth-Moon scene
- Full trajectory path visible as a 3D curve
- Spacecraft model follows the trajectory in real time (with time scaling)
- Mission events labeled in the scene
- Camera can follow the spacecraft or stay fixed on Earth/Moon

**Stretch:**
- Show the translunar injection burn as a velocity vector change
- Add a 2D ground track panel showing the trajectory projected onto Earth
- Support loading multiple missions from different data files (dropdown selector)
- Add Apollo 13 free-return trajectory as a second dataset for comparison

> **Claude's note:** The cubic Hermite interpolation using position + velocity is the right call — linear interpolation of orbital trajectories looks jagged, especially near the Moon where curvature is high. If SPICE kernels are too painful to wrangle, JPL Horizons has a web interface that can export state vectors as plain text — much easier to parse. For the mission event markers, consider small billboard sprites with labels rather than 3D geometry — they should face the camera and remain readable at any zoom level.

---

### *** Milestone: Visual Foundation Complete ***

At the end of Phase 3, the project is a real-data solar system visualizer with a playable NASA mission. This is already a strong portfolio piece — it shows OpenGL rendering, real-world data handling, coordinate systems, time management, and domain knowledge. Everything after this adds engineering depth.

---

## Part II — Engineering Depth

---

### Phase 4 — Analytical Dashboard

**8-12 hours | ~2-3 weeks**

**Goal:** Add engineering-grade analytical panels alongside the 3D view. The application stops looking like a game and starts looking like mission control software.

**Layout:** `QMainWindow` with `QDockWidget` panels. The 3D view becomes one dock among several. The analytical widgets carry equal weight.

**Tasks:**
- Refactor `MainWindow` to use `QDockWidget` layout with saveable/restorable arrangement
- **Ground Track Widget:** 2D equirectangular map (`QPainter`) showing spacecraft lat/lon trace from ECI->ECEF
- **Orbital Elements Widget:** real-time display of classical orbital elements (a, e, i, RAAN, omega, nu) computed from state vectors
- **Telemetry Charts Widget:** plots of altitude, velocity, specific orbital energy vs time (QCustomPlot or Qt Charts)
- **Control Panel Widget:** consolidate existing controls + mission selector + perturbation toggles (for later phases)
- Ensure 60 FPS UI while all panels update

**Deliverables:**
- Multi-panel application: 3D view + ground track + charts + orbital elements + controls
- All panels update in sync with the simulation clock
- Dockable, rearrangeable layout
- Screenshot-ready for README

**What This Demonstrates:**
Qt proficiency beyond basic widgets, professional UI architecture, data visualization, domain-appropriate engineering displays.

---

### Phase 5 — Eigen + RK4 + Two-Body Propagation

**8-10 hours | ~2-3 weeks**

**Goal:** Build a custom orbit propagator using Eigen and a Runge-Kutta integrator. The application can now compute orbits, not just play back recorded ones. Compare propagated trajectories against the real mission data already in the visualizer.

**Why Eigen:**
Industry standard for linear algebra in aerospace C++. Using it signals professional practice. The RK4 integrator is the workhorse of orbital mechanics — getting it right, with energy conservation validation, proves numerical analysis competence.

**Tasks:**
- Integrate Eigen3 via CMake `find_package` or `FetchContent`
- Define core types: `using Vector3d = Eigen::Vector3d;` `using StateVector = Eigen::Matrix<double, 6, 1>;`
- Implement `Integrator` interface with virtual `step()` — takes a generic `f(state, t)` callable
- Implement RK4 integrator for general ODE: dx/dt = f(x, t)
- Implement two-body gravity: **a = -mu/r^3 * r**
- Propagate orbits and render them in the 3D scene as computed trajectory curves
- **Validation:** propagate Artemis II initial conditions with two-body and overlay against the real trajectory — the divergence is visible and educational (shows why perturbations matter)
- Unit tests: energy conservation over 10+ orbit periods, comparison against analytical Keplerian solution

**Deliverables:**
- Custom orbit propagator producing trajectories in the 3D scene
- Propagated vs real trajectory comparison visible in the visualizer
- Energy conservation validated over long propagation windows
- Integrator architecture supports drop-in replacement (Dormand-Prince later)

> **Claude's note:** The "propagated vs real" overlay is a killer feature. It immediately shows why two-body isn't enough (the trajectories diverge near the Moon), which motivates Phase 6's perturbations in a way that's visually obvious. Keep the integrator callable-based — `step(f, state, t, dt)` rather than baking in the force model — so adding perturbations in Phase 6 is just composing acceleration functions.

---

### Phase 6 — Perturbation Physics: J2 + Atmospheric Drag

**8-10 hours | ~2-3 weeks**

**Goal:** Add the two most important real-world perturbations. This is aerospace domain knowledge that a generic software engineer cannot fake.

**Why This Matters:**
J2 (Earth oblateness) drives sun-synchronous orbit design — bread and butter for companies like ICEYE and Planet. Atmospheric drag causes LEO orbit decay. Implementing both shows understanding of the physics that space companies deal with daily. And with the real trajectory data from Phase 3, the improvement in accuracy is directly visible.

**Tasks:**
- Implement J2 perturbation acceleration (zonal harmonic formulation)
- Implement exponential atmospheric drag model: **F_drag = -1/2 rho C_d A v^2 v_hat**
- Create `PropagatorConfig` to toggle perturbations on/off at runtime
- Implement ECI to ECEF transformation (Earth rotation)
- Validate J2: verify nodal precession rate matches analytical formula
- Validate drag: verify semi-major axis decay at ISS altitude is physically reasonable
- **Visual payoff:** re-propagate with perturbations enabled, show improved match against real Artemis II data
- Benchmark: cost per step with perturbation combinations

**Deliverables:**
- Visibly different orbits with J2 (precessing orbit plane)
- Drag-induced orbit decay visible in accelerated time
- Propagated trajectory closer to real data with perturbations enabled
- Benchmark documented

---

### Phase 7 — Monte Carlo Concurrency Engine

**10-15 hours | ~3-4 weeks**

**Goal:** The senior-engineer showcase. Given initial state uncertainty, run thousands of parallel orbit propagations and visualize the resulting state dispersion. This is a real mission analysis problem solved with modern C++ concurrency.

**Tasks:**
- Design thread pool using `std::jthread` with cooperative cancellation (`std::stop_token`)
- Gaussian perturbation sampling: nominal state + covariance matrix -> N perturbed initial conditions
- Distribute 10,000+ propagations across CPU cores
- Lock-free aggregation: compute mean state and covariance from completed runs
- Visualize dispersion: uncertainty ellipse on ground track, error envelope on trajectory, error bars on charts
- Benchmark: wall-clock time vs core count, demonstrate near-linear scaling
- `-fsanitize=thread` clean

**Deliverables:**
- 10,000+ parallel propagations running on the Monte Carlo engine
- Dispersion visualization overlaid on the dashboard
- Scaling benchmark with documented throughput graph
- Thread sanitizer clean

**What This Demonstrates:**
C++20 concurrency (`jthread`, `stop_token`), thread pool design, lock-free aggregation, statistical computing, performance profiling — the kind of analysis actual mission planning teams perform.

> **Claude's note:** With the Artemis II trajectory as context, Monte Carlo dispersion becomes tangible: "given sensor noise in the TLI burn, how much does the lunar closest approach distance vary?" That's a real mission analysis question. Showing the dispersion envelope around the Moon flyby in the 3D scene is visually stunning and technically deep.

---

## Part III — Polish

---

### Phase 8 — Testing, CI, and Portfolio Polish

**6-8 hours | ~2 weeks (overlaps other phases)**

**Goal:** Turn good code into a standout portfolio artifact.

**Tasks:**
- Integrate GoogleTest via `FetchContent`; tests per module: ephemeris accuracy, integrator validation, coordinate transforms, interpolation, thread safety
- GitHub Actions: build on Ubuntu (GCC), run tests, `clang-format` check
- Sanitizers in CI: `-fsanitize=address` and `-fsanitize=thread`
- README: architecture diagram, module dependency graph, threading model, physics validation, design tradeoffs
- 60-second demo video: Earth-Moon scene, Artemis II playback, time controls, Monte Carlo dispersion
- "What This Demonstrates" section mapping implementation choices to engineering competencies

**Deliverables:**
- CI green on every push, badge in README
- README that works for hiring managers and senior engineers simultaneously
- Demo video linked from README

---

## What's Deliberately Cut (and Why)

| Cut Item | Reason |
|----------|--------|
| VTK | Replaced with custom OpenGL — more control, more impressive, already done. |
| Hand-rolled Vector3 / Quaternion | Eigen does this better. Professional practice, not reinvention. |
| SGP4 / live TLE fetching | Integration work, not engineering. A custom propagator validated against real data is more impressive. |
| Attitude dynamics / telemetry engine | Separate domain from orbital mechanics. Depth over breadth. |
| spdlog / nlohmann::json / config system | Infrastructure that consumes hours without demonstrating skill. |
| SPICE runtime dependency | Pre-compute trajectory data offline. Ship the data, not the dependency. |
| Windows / MSVC | Ubuntu + GCC is sufficient for portfolio purposes. |

> **Claude's note:** One item I'd add back post-Phase 8 if there's appetite: attitude visualization for the Artemis II spacecraft (nadir-pointing, velocity-aligned). Once the trajectory is in place it's cheap to add and visually satisfying. But it's strictly optional.

---

## Portfolio Signal Summary

| Competency | Evidence |
|---|---|
| Custom OpenGL | Hand-written shaders, Blinn-Phong lighting, logarithmic depth buffer, camera-relative rendering, orbit trails |
| Real-World Data | NASA mission trajectory, analytical ephemeris, Julian Date time system |
| Aerospace Domain | ECI/ECEF frames, orbital elements, J2 precession, atmospheric drag, mission analysis |
| Modern C++ (C++20) | `std::jthread`, `stop_token`, concepts, move semantics, RAII |
| Numerical Analysis | RK4 integrator, energy conservation, Hermite interpolation, Meeus algorithms |
| Concurrency | Thread pool, Monte Carlo parallelism, lock-free aggregation, sanitizer-clean |
| Systems Architecture | Clean module boundaries, Eigen integration, runtime-configurable propagator |
| Data Visualization | Ground track, telemetry charts, dispersion ellipses, mission playback |
| Qt Proficiency | QDockWidget dashboard, QOpenGLWidget, custom painting, responsive threading |
| Testing & CI | GoogleTest, GitHub Actions, sanitizers, physics validation |

> A serious C++ engineer who builds real-world aerospace tools — from the pixels on screen to the physics underneath.

---

## Suggested Weekly Rhythm

**Session A (2 hours, weekday evening):** Write code. Implement the next task. Commit and push before stopping.

**Session B (1.5-3 hours, weekend):** Test, validate, document. Write unit tests for Session A's work. Update README if a deliverable is complete.

Never end a session with uncommitted work. Small, frequent commits with clear messages are themselves a portfolio signal.
