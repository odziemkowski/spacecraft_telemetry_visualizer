
---

# The 2-Week MVP Plan (Absolutely Achievable)

## Week 1: Core Visualization (Visible Progress Immediately)

**Goal:** Display a 3D spacecraft model and rotate it

---

### Day 1 (2 hours) – Saturday morning

* Create GitHub repo
* Set up Qt + VTK project (`CMakeLists.txt`)
* Display empty VTK window in Qt

**Deliverable:** Window opens successfully
**Commit:** `"Initial project setup"`

---

### Day 2 (2 hours)

* Load a simple 3D model (cube or sphere)
* Display in VTK window
* Add basic mouse rotation

**Deliverable:** You can see and rotate 3D object
**Commit:** `"Add 3D model display"`

---

### Day 3–4 (1 hour each)

* Find/create simple satellite 3D model (OBJ file)
* Load and display satellite
* Add axes (X, Y, Z)

**Deliverable:** Satellite visible with orientation axes
**Commit:** `"Add satellite model"`

---

### Day 5 (2 hours)

* Add simple UI with Qt (sliders or buttons)
* Control satellite rotation with UI

**Deliverable:** Interactive satellite rotation
**Commit:** `"Add UI controls"`

---

## Weekend 1 End

* ✅ You have a working 3D satellite viewer
* ✅ It's on GitHub
* ✅ It looks cool (screenshot-worthy)
* ✅ Momentum established

---

# Week 2: Add Real Data (Make It Functional)

**Goal:** Read telemetry data and update visualization

---

### Day 6–7 (2 hours each)

* Create sample telemetry CSV file (attitude, position, temperature, etc.)
* Parse CSV data in C++

**Deliverable:** Program reads telemetry file
**Commit:** `"Add telemetry parsing"`

---

### Day 8–9 (2 hours each)

* Update satellite orientation from telemetry data
* Add "playback" – step through telemetry timeline

**Deliverable:** Satellite moves according to data
**Commit:** `"Implement telemetry playback"`

---

### Day 10 (2 hours)

* Add simple telemetry display (text showing current values)
* Polish UI
* Take screenshots/video

**Deliverable:** Demo-ready project
**Commit:** `"Add telemetry display UI"`


Absolutely. Below is a **realistic 3-month roadmap** designed for:

* ✅ Someone with solid programming skills
* ✅ Limited system design & multithreading experience
* ✅ Desire to grow into strong modern C++
* ✅ Aerospace flavor without pigeonholing
* ✅ Fun + visible progress
* ✅ Portfolio-ready result

It is structured for **~6–8 hours per week** (adjust pace as needed).

You can paste this directly into GitHub.

---

# 🛰 Satellite Telemetry & Simulation Workbench

### 3-Month Engineering Roadmap

---

# 📅 Month 1 – Architecture & Mathematical Core

**Theme:** Build strong foundations
**Focus:** Clean structure, math correctness, API thinking
**Outcome:** A clean, modular project with a working math + telemetry core

---

## Week 1 – Project Restructure & Boundaries

### Goals

* Refactor MVP into clear modules
* Separate UI, visualization, and core logic
* Introduce modern C++ ownership semantics

### Tasks

* Create folder structure:

```
/src
  /core
  /math
  /simulation
  /visualization
  /ui
/tests
```

* Ensure:

  * No Qt in `/core`
  * No VTK in `/math`
  * Core has no UI dependency
* Replace raw pointers with:

  * `std::unique_ptr`
  * `std::shared_ptr` (only if needed)
* Add `const` correctness where missing

### Deliverables

* Clean build
* Clear module boundaries
* Updated README architecture section

---

## Week 2 – Implement Math Module

### Goals

Build a small but solid math foundation.

### Implement

* `Vector3`

  * Addition, subtraction
  * Scalar multiplication
  * Dot product
  * Cross product
  * Norm / normalization

* `Quaternion`

  * Constructors
  * Normalize
  * Multiplication
  * Conjugate
  * Rotate vector
  * Convert to rotation matrix

### Stretch

* Implement SLERP

### Rules

* No dynamic allocation
* Value semantics only
* Use `constexpr` where appropriate
* Add `explicit` constructors

### Deliverables

* `math` module independent of UI
* Unit tests for vector + quaternion
* README: Math design notes

---

## Week 3 – Telemetry Engine (API Design Practice)

### Goals

Move from “file reader” to “telemetry system”.

### Design

Create:

```cpp
struct SatelliteState {
    TimePoint timestamp;
    Vector3 position;
    Quaternion attitude;
};
```

Create:

```cpp
class TelemetryEngine {
public:
    bool load_from_csv(const std::string& path);
    std::optional<SatelliteState> state_at(TimePoint t) const;
};
```

### Features

* Store telemetry ordered by timestamp
* Support interpolation between samples
* Use `std::chrono` for time
* Use `std::optional` for safe querying

### Deliverables

* Clean API
* Interpolation working
* Unit tests for interpolation edge cases

---

## Week 4 – Integration Layer

### Goals

Connect telemetry engine to visualization.

### Tasks

* Replace direct CSV playback with `TelemetryEngine`
* Update satellite orientation via quaternion
* Ensure clean separation:

  * Visualization consumes state
  * Core produces state

### Deliverables

* Playback working with new architecture
* Code review pass (self-review for clarity)
* README: Architecture diagram

---

# 📅 Month 2 – Concurrency, Simulation & System Thinking

**Theme:** Move toward real engineering
**Focus:** Threading, simulation logic, design clarity
**Outcome:** Concurrent telemetry pipeline + simulation engine

---

## Week 5 – Introduction to Multithreading

### Goals

Add a simple background worker.

### Implement

* Create a telemetry playback thread
* Use:

  * `std::thread`
  * `std::mutex`
  * `std::lock_guard`

Architecture:

```
Telemetry Thread → Shared State → UI Thread
```

### Rules

* No global variables
* Clearly document shared ownership
* Use RAII for thread lifecycle

### Deliverables

* Threaded playback
* Clean shutdown
* README: Threading model explanation

---

## Week 6 – Improve Threading Design

### Goals

Make threading safer and more structured.

### Refactor to:

* `std::jthread` (if C++20)
* Add:

  * `std::condition_variable`
  * Producer-consumer model

Optional:

* Implement simple thread-safe queue

### Deliverables

* Stable concurrent pipeline
* No data races (test heavily)
* README: Concurrency design notes

---

## Week 7 – Attitude Simulation Engine

### Goals

Add real math-based simulation.

### Implement

Basic rigid-body propagation:

* Angular velocity integration
* Quaternion update:

```
q_dot = 0.5 * q ⊗ omega
```

* Normalize periodically

Optional:

* Add noise injection
* Simulate sensor drift

### Deliverables

* Simulated mode vs telemetry mode
* Toggle in UI
* Unit tests for propagation

---

## Week 8 – Time-Series Enhancements

### Goals

Improve data handling maturity.

### Add

* Ring buffer for live simulation
* Time scaling (2x, 0.5x)
* Pause/resume
* Step frame

Optional:

* Error comparison between simulated & real telemetry

### Deliverables

* Smooth playback
* Improved performance
* Basic profiling using `std::chrono`

---

# 📅 Month 3 – Professional Polish & Engineering Depth

**Theme:** Make it portfolio-grade
**Focus:** Testing, performance, documentation, realism
**Outcome:** A resume-worthy engineering tool

---

## Week 9 – Testing Discipline

### Goals

Increase credibility.

### Add tests for:

* Quaternion edge cases
* Interpolation boundaries
* Telemetry parsing errors
* Thread safety scenarios

Integrate:

* GoogleTest or Catch2
* GitHub Actions CI

### Deliverables

* Automated test pipeline
* Tests running on every push

---

## Week 10 – Logging & Configuration

### Goals

Make it feel like real software.

### Add

* Logging system (e.g., spdlog)

  * Log levels
  * File output
* JSON configuration file

  * Model path
  * Playback speed
  * Simulation parameters

### Deliverables

* Config-driven startup
* Structured logs
* Updated README

---

## Week 11 – Performance & Memory Awareness

### Goals

Demonstrate systems thinking.

### Improve

* Avoid unnecessary copies
* Use move semantics intentionally
* Profile telemetry parsing
* Review memory ownership

Optional:

* Convert telemetry storage to struct-of-arrays layout

### Deliverables

* Document performance considerations
* Add benchmark notes to README

---

## Week 12 – Final Polish & Presentation

### Goals

Turn it into a standout portfolio piece.

### Add

* Clean README with:

  * Architecture diagram
  * Threading model diagram
  * Math explanation
  * Design tradeoffs
* Screenshots / demo video
* Feature list
* Future roadmap section

Optional:

* Add ground track visualization
* Add ECI/ECEF frame conversion

### Final Outcome

You now have:

* Modular architecture
* Custom math library
* Telemetry engine
* Concurrent pipeline
* Simulation engine
* Testing suite
* CI integration
* Logging & configuration
* Professional documentation

---

# 🏁 Final Portfolio Positioning

This project demonstrates:

* Modern C++ (C++17/20)
* Concurrency
* Mathematical reasoning
* System design fundamentals
* Data pipeline thinking
* Performance awareness
* Engineering discipline
* Aerospace domain familiarity

It positions you as:

> A serious C++ engineer capable of building technical systems — not just UI features.

---

# 📌 Optional README Section

You may add:

```
## Learning Objectives

This project was intentionally designed to deepen knowledge in:
- System architecture
- Multithreading in C++
- Numerical stability in quaternion math
- Time-series data handling
- Modern C++ best practices
```

---

If you'd like next, I can:

* Help you design the initial folder + CMake template
* Draft a professional README
* Or design a milestone-based GitHub issue breakdown for easier tracking
