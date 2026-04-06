# OpenGL Concepts & Workflow

A guide to understanding OpenGL for rendering the spacecraft telemetry visualizer.

## Core Concepts

### 1. **Vertices and Geometry**
- **Vertex**: A single point in 3D space with associated data (position, normal, texture coordinate, color, etc.)
- **Mesh**: A collection of vertices that form a 3D object (e.g., a sphere, satellite, cube)
- **Index**: A reference to a vertex used to build triangles efficiently

### 2. **GPU Buffers**

OpenGL requires data to live on the GPU for fast rendering. We use three types of buffers:

#### **VBO (Vertex Buffer Object)**
- Stores vertex data on the GPU (positions, normals, texture coordinates)
- Created once during setup, reused every frame
- Example: `m_earthVBO` holds all the vertex data for the Earth sphere

#### **EBO (Element Buffer Object)** / **IBO (Index Buffer Object)**
- Stores indices that reference vertices to form triangles
- Avoids duplicate vertex data by reusing vertices
- Example: `m_earthEBO` tells OpenGL which vertices form each triangle of the sphere

#### **VAO (Vertex Array Object)**
- **Captures the vertex layout blueprint** — stores configuration of how vertex data is organized
- Remembers: "position data is here, normals are here, texture coords are here"
- Also captures the currently-bound EBO
- Think of it as a "recipe card" that says "here's how to interpret the vertex data"

### 3. **Shaders**

Programs that run on the GPU to process vertices and determine pixel colors.

#### **Vertex Shader**
- Runs once per vertex
- Typical jobs: transform vertices to screen space, compute lighting per vertex, pass data to fragment shader
- Example: `kLitVertexShader` transforms vertices and computes normal vectors

#### **Fragment Shader** (Pixel Shader)
- Runs once per pixel
- Typical jobs: apply textures, compute final color, apply lighting
- Example: `kLitFragmentShader` samples the Earth texture and applies directional lighting

#### **Shader Program**
- Combines vertex + fragment shaders into a single executable program
- Example: `m_litShader` is the program used for textured, lit objects

### 4. **Textures**

Images uploaded to the GPU that can be sampled by shaders.

- **Texture Handle**: A GPU identifier (e.g., `m_earthTexture`)
- **Texture Sampling**: Reading a color from a texture based on coordinates (U, V coordinates)
- **Texture Unit**: A "slot" on the GPU that holds an active texture (e.g., unit 0, unit 1)

---

## Typical OpenGL Workflow

### **Setup Phase** (happens once at initialization)

```
1. Generate mesh geometry (vertices, normals, indices)
2. Create VAO
3. Bind VAO
4. Create VBO and upload vertex data
5. Set up vertex attributes (position, normal, texcoord)
6. Create EBO and upload index data
7. Release VAO
8. Create shader programs (compile vertex + fragment shaders)
9. Load and upload textures to GPU
```

**Result**: GPU now has geometry, shaders, and textures ready to go.

### **Rendering Loop** (happens every frame in `paintGL()`)

```
1. Bind shader program
2. Set shader uniforms (matrices, light direction, colors, etc.)
3. Bind texture
4. Bind VAO (restores vertex layout + EBO)
5. Call glDrawElements() to render
6. Release VAO and texture
```

**What happens inside `glDrawElements()`**:
- GPU reads indices from EBO
- For each vertex, GPU runs vertex shader
- For each pixel, GPU runs fragment shader
- Shader samples texture and computes final color
- Result appears on screen

---

## How It Works Together

### Data Flow for Rendering the Earth:

```
setupEarth():
  ├─ Generate sphere vertices → store in CPU memory
  ├─ Create VBO → upload vertices to GPU
  ├─ Create EBO → upload indices to GPU
  ├─ Create VAO → capture "how to read the VBO" (positions are 3 floats, then normals, then texcoords)
  └─ Load texture → upload Earth image to GPU

paintGL():
  ├─ glBindTexture() → activate Earth texture
  ├─ m_earthVAO.bind() → tell GPU "use this vertex layout"
  ├─ m_litShader->bind() → activate shader program
  ├─ setUniformValue() → pass matrices & light info to shader
  ├─ glDrawElements() → GPU executes:
  │   ├─ Vertex shader runs for each vertex:
  │   │   └─ Transform position to screen space
  │   └─ Fragment shader runs for each pixel:
  │       ├─ Read texture color at this pixel's texcoord
  │       ├─ Compute lighting
  │       └─ Output final color
  └─ Result: Earth sphere appears on screen
```

---

## Key Points

1. **VAO is a blueprint**: It doesn't store actual data; it stores *how to interpret* the data in VBO/EBO
2. **Texture is separate**: Unlike the VAO, the texture is actual image data, stored independently
3. **Bind/Release pattern**: You bind objects to make them active, release them when done
4. **GPU memory persists**: Once uploaded, data stays on GPU until explicitly deleted, ready to reuse
5. **Shaders define appearance**: The same geometry can look completely different with different shaders

---

## Common OpenGL Objects in This Project

| Object | Type | Purpose |
|--------|------|---------|
| `m_earthVAO` | VAO | Vertex layout for Earth sphere |
| `m_earthVBO` | VBO | Vertex data (positions, normals, texcoords) for Earth |
| `m_earthEBO` | EBO | Triangle indices for Earth |
| `m_earthTexture` | Texture | Earth image (bluemarble_2k.jpg) |
| `m_litShader` | Shader Program | Rendering with lighting & textures |
| `m_unlitShader` | Shader Program | Simple color rendering (axes, markers) |
| `m_satVAO`, `m_satVBO`, `m_satEBO` | VAO/VBO/EBO | Satellite mesh |
| `m_axesVAO`, `m_axesVBO` | VAO/VBO | ECI coordinate axes (unlit) |

---

## Debugging Tips

- **Objects not appearing?** Check:
  - Is the VAO bound before drawing?
  - Is the shader program bound?
  - Does the index count match the actual data?
  - Is the object within the camera's view frustum?

- **Texture not showing?** Check:
  - Did glBindTexture() succeed?
  - Does the fragment shader use the texture?
  - Are texture coordinates correct?

- **Wrong colors?** Check:
  - Fragment shader code
  - Texture format (RGB vs RGBA)
  - Lighting calculations
