# The Complete Rendering Pipeline

How OpenGL fits into a real application: from user input to pixels on your screen.

---

## The Big Picture: Application Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    APPLICATION WINDOW                       │
│  (Qt QOpenGLWidget - your GLViewport class)                 │
└─────────────────────────────────────────────────────────────┘
                           ▲
                           │
        ┌──────────────────┼──────────────────┐
        │                  │                  │
    Input Loop        Rendering Loop     Logic Loop
    (Mouse/Keys)      (paintGL)           (Updates)
        │                  │                  │
        ▼                  ▼                  ▼
   ┌─────────┐    ┌──────────────┐    ┌──────────────┐
   │ Events  │    │ Render Frame │    │ Update State │
   │ Handler │    │   to Screen  │    │  & Physics   │
   └─────────┘    └──────────────┘    └──────────────┘
```

---

## The 3D Scene

Your application has a **3D world** with objects:

```
3D World Space (Infinite coordinate system)
├─ Earth (at origin, 1 unit radius)
├─ Satellite (orbiting Earth, rotating)
├─ Sun (direction indicator)
├─ Coordinate Axes (X, Y, Z at origin)
└─ Orientation Marker (on satellite)

Each object has:
- Position (X, Y, Z)
- Rotation (roll, pitch, yaw or quaternion)
- Scale
- Material (color, texture)
```

---

## The Camera

The camera is your "eye" looking at the 3D world.

```
3D World Space
        │
        │ The camera decides what portion of
        │ the world you see
        │
    ┌───┴───┐
    │ Camera │  Position: (10, 5, 8)
    │ (Eye)  │  Looking at: (0, 0, 0) - the Earth
    └───┬───┘  Up: (0, 1, 0)
        │
        │ What the camera sees (view frustum)
        │
    ┌───┴──────────┐
    │  View Volume │  Only objects in here are rendered
    │              │
    └───┬──────────┘
        │
        ▼
    Screen (2D projection)
```

**View Matrix**: Transforms world coordinates → camera-relative coordinates
- Essentially: "Where is the camera and what is it looking at?"
- If camera moves left, objects appear to move right (parallax)

**Projection Matrix**: Transforms camera view → 2D screen coordinates
- Handles perspective (closer objects larger)
- Defines the "field of view" (narrow vs. wide angle)

---

## From 3D World to 2D Screen: The Transformation Pipeline

This is **the magic** that happens for every vertex:

```
┌──────────────────────────────────────────────────────────────┐
│ Vertex in 3D World Space                                     │
│ Example: Satellite at position (5, 2, -3)                   │
└──────────────────────────────────────────────────────────────┘
                           │
                           ▼
         ╔════════════════════════════════════════╗
         ║   Model Matrix (Object Transform)     ║
         ║   - Positions the satellite in space  ║
         ║   - Rotations (roll, pitch, yaw)      ║
         ║   - Scaling                           ║
         ║   Result: World space coordinates     ║
         ╚════════════════════════════════════════╝
                           │
                           ▼
         ╔════════════════════════════════════════╗
         ║   View Matrix (Camera Transform)      ║
         ║   - Where is the camera?              ║
         ║   - What is it looking at?            ║
         ║   Result: Camera-relative coords      ║
         ╚════════════════════════════════════════╝
                           │
                           ▼
         ╔════════════════════════════════════════╗
         ║ Projection Matrix (Perspective)       ║
         ║   - Apply perspective (closer=bigger) ║
         ║   - Clip objects outside view         ║
         ║   Result: Normalized device coords    ║
         ╚════════════════════════════════════════╝
                           │
                           ▼
┌──────────────────────────────────────────────────────────────┐
│ 2D Screen Pixel Coordinates                                  │
│ Example: Pixel (640, 360) on your monitor                   │
└──────────────────────────────────────────────────────────────┘
```

**This transformation happens in the vertex shader:**
```glsl
vec4 worldPos = model * vec4(aPos, 1.0);        // Model matrix
vec4 viewPos = view * worldPos;                  // View matrix
gl_Position = projection * viewPos;              // Projection matrix
```

---

## How Movement Works

### **Camera Movement** (Orbit Camera)

In your visualizer, the camera orbits around the Earth:

```cpp
// User moves mouse
mouseMoveEvent() receives input
  ├─ Calculate mouse delta
  ├─ Update camera angles (yaw, pitch)
  └─ Recompute view matrix

// Next frame
paintGL() called
  ├─ View matrix reflects new camera position
  ├─ All objects transformed with new view matrix
  └─ Screen shows scene from new camera angle
```

**Result**: Objects appear to move on screen as you drag.

### **Object Movement** (Satellite Rotation)

In your visualizer, the satellite rotates:

```cpp
// Application logic
setSatelliteOrientation(roll, pitch, yaw)
  └─ Update m_satRoll, m_satPitch, m_satYaw

// Next frame
paintGL() called
  ├─ Reconstruct model matrix from rotation angles
  ├─ Apply model matrix to satellite vertices
  ├─ Vertices are now in new rotated position
  └─ Screen shows satellite in new orientation
```

**Result**: Satellite rotates smoothly frame by frame.

---

## The Rendering Loop (paintGL)

This is called ~60 times per second:

```
paintGL() {
    1. Clear screen (black background)
    
    2. Update view matrix based on camera position
       view = camera.getViewMatrix();  // Camera moved?
    
    3. For EACH object in scene:
    
       a. Compute model matrix
          model = translate * rotate * scale;
          
       b. Bind shader program
       
       c. Set uniforms
          shader->setUniformValue("model", model);
          shader->setUniformValue("view", view);
          shader->setUniformValue("projection", projection);
          shader->setUniformValue("lightDir", lightDirection);
          
       d. Bind VAO and texture
       
       e. glDrawElements()
          GPU executes vertex shader for each vertex:
            - Transforms vertex: projection * view * model * vertex
            - Passes normal, texcoord to fragment shader
          GPU executes fragment shader for each pixel:
            - Samples texture
            - Computes lighting
            - Outputs color
    
    4. Swap buffers (double buffering)
       Old screen shown while new frame renders
}
```

**Key insight**: The view and model matrices are computed *every frame* based on current state.

---

## Complete Data Flow: User Clicks & Drags

```
User Input
    │
    ▼
┌─────────────────────────┐
│ mouseMoveEvent()        │  ← Qt calls this
│ mouse.x = 650, y = 300  │
└─────────────────────────┘
    │
    ▼
┌─────────────────────────┐
│ Update camera state     │
│ m_camera.rotate(...)    │
│ Camera angles changed   │
└─────────────────────────┘
    │
    ▼ (next event loop iteration)
┌─────────────────────────┐
│ paintGL() called        │
└─────────────────────────┘
    │
    ├─ Compute new view matrix from camera angles
    │  view = camera.getViewMatrix()
    │
    ├─ For Earth:
    │  ├─ model = identity (Earth at origin)
    │  ├─ Bind Earth shader, texture, VAO
    │  ├─ Pass matrices to shader
    │  └─ glDrawElements() → Earth rendered
    │
    ├─ For Satellite:
    │  ├─ model = translate(-5, 0, 0) * rotate(m_satRoll, m_satPitch, m_satYaw)
    │  ├─ Bind Sat shader, VAO
    │  ├─ Pass matrices to shader
    │  └─ glDrawElements() → Satellite rendered
    │
    └─ All objects now rendered with new camera angle
       ▼
   Screen updates
   → User sees Earth and satellite from new viewpoint
```

---

## In Your Spacecraft Telemetry Visualizer

### **Scene Objects**
- **Earth**: Textured sphere, static position, lighting enabled
- **Satellite**: 3D model, position controlled by telemetry, rotation controlled by `setSatelliteOrientation()`
- **Coordinate Axes**: Three lines (X, Y, Z), unlit, help understand orientation
- **Sun Indicator**: Shows light direction, unlit
- **Orientation Marker**: Visual aid on satellite, unlit

### **Matrices in Your Code**

```cpp
// View Matrix (camera)
QMatrix4x4 view = m_camera.getViewMatrix();

// Model Matrices (objects)
QMatrix4x4 modelEarth;  // Identity - Earth at origin

QMatrix4x4 modelSat;
modelSat.translate(-5, 0, 0);  // Position in orbit
modelSat.rotate(m_satRoll, 1, 0, 0);   // Roll
modelSat.rotate(m_satPitch, 0, 1, 0);  // Pitch
modelSat.rotate(m_satYaw, 0, 0, 1);    // Yaw

// Projection Matrix (perspective)
QMatrix4x4 projection;
projection.perspective(45.0f, aspectRatio, 0.1f, 100.0f);
```

### **When You Move the Mouse**
1. `mouseMoveEvent()` updates `m_camera`
2. Next `paintGL()`, new view matrix is computed
3. All objects transformed with new view matrix
4. Objects appear to move on screen (actually camera moved)

### **When Satellite Orientation Changes**
1. Telemetry updates call `setSatelliteOrientation(roll, pitch, yaw)`
2. Member variables `m_satRoll`, `m_satPitch`, `m_satYaw` change
3. Next `paintGL()`, new model matrix is built from these angles
4. Satellite appears to rotate

---

## The Frame-by-Frame Cycle

```
While Application Running:
    ├─ Handle Input Events
    │  └─ Update camera/state based on user actions
    │
    ├─ Update Logic
    │  └─ Update satellite position/orientation from telemetry
    │
    ├─ Render Frame (paintGL)
    │  ├─ Compute matrices (view, model, projection)
    │  ├─ For each object: bind, set uniforms, draw
    │  └─ GPU processes vertices & fragments
    │
    └─ Display on Screen
       (Repeat 60x per second)
```

---

## Key Takeaways

1. **The camera is relative**: Moving the camera is equivalent to transforming the entire world oppositely
2. **Matrices are state**: View and model matrices encode the current state each frame
3. **GPU is fast**: The vertex/fragment shader runs *millions* of times per frame—that's why we use GPU
4. **Double buffering**: One frame renders while the previous displays (smooth, no flicker)
5. **Everything is transformation**: 3D graphics is fundamentally about transforming coordinates through matrices
6. **Shaders define appearance**: Same vertex data, different shaders = completely different look

---

## Questions You Might Have

**Q: Why do we recompute the view matrix every frame if the camera didn't move?**
A: Because it's cheap! Matrix multiplication is extremely fast on GPU. Checking "did camera move?" costs more than just recomputing.

**Q: What if I have 1000 objects? Does rendering get 1000x slower?**
A: Roughly yes. This is why games use techniques like:
- Frustum culling (don't render objects outside view)
- Instancing (render many identical objects in one call)
- Level of detail (simplified geometry far away)

**Q: Why are there two shader programs (lit and unlit)?**
A: Different objects need different visual treatment. Earth needs lighting and texture; axes just need a color.

**Q: Could I animate objects differently? Orbit? Bounce?**
A: Yes! You'd compute the model matrix differently. Example:
```cpp
float angle = time * speed;  // Rotate with time
modelSat.translate(cos(angle) * 5, 0, sin(angle) * 5);  // Circular orbit
```

