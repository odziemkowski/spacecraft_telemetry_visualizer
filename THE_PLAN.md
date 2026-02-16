
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
