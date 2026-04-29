#include "GLViewport.h"
#include "Geometry.h"
#include "ObjLoader.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QMouseEvent>
#include <QWheelEvent>

// --- Shader sources ---

// Lit shader program: For objects with directional lighting (Earth, satellite).
// Vertex stage: Transforms vertices to world/view/projection space, computes normal matrix for lighting.
// Fragment stage: Implements directional lighting with ambient + diffuse components, supports texture or solid color.

static const char *kLitVertexShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform float u_far;

out vec3 fragNormal;
out vec3 fragPos;
out vec2 texCoord;
out float flogz;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    fragPos = worldPos.xyz;
    fragNormal = normalMatrix * aNormal;
    texCoord = aTexCoord;
    gl_Position = projection * view * worldPos;
    flogz = 1.0 + gl_Position.w;
    gl_Position.z = (log2(max(1e-6, flogz)) * (2.0 / log2(u_far + 1.0)) - 1.0) * gl_Position.w;
}
)";

static const char *kLitFragmentShader = R"(
#version 330 core
in vec3 fragNormal;
in vec3 fragPos;
in vec2 texCoord;
in float flogz;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform vec3 materialColor;
uniform bool hasTexture;
uniform sampler2D textureSampler;
uniform float u_far;

out vec4 fragColorOut;

void main()
{
    vec3 norm = normalize(fragNormal);
    float diff = max(dot(norm, normalize(lightDir)), 0.0);
    vec3 baseColor = hasTexture ? texture(textureSampler, texCoord).rgb : materialColor;
    vec3 result = (ambientColor + diff * lightColor) * baseColor;
    fragColorOut = vec4(result, 1.0);
    gl_FragDepth = log2(max(1e-6, flogz)) / log2(u_far + 1.0);
}
)";

// Unlit shader program: For simple geometry without lighting (coordinate axes, sun indicator, markers).
// Vertex stage: Simple pass-through transformation using combined model-view-projection matrix.
// Fragment stage: Outputs a uniform color with no lighting calculations.

static const char *kUnlitVertexShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 mvp;
uniform float u_far;

out float flogz;

void main()
{
    gl_Position = mvp * vec4(aPos, 1.0);
    flogz = 1.0 + gl_Position.w;
    gl_Position.z = (log2(max(1e-6, flogz)) * (2.0 / log2(u_far + 1.0)) - 1.0) * gl_Position.w;
}
)";

static const char *kUnlitFragmentShader = R"(
#version 330 core
in float flogz;

uniform vec3 color;
uniform float u_far;

out vec4 fragColorOut;

void main()
{
    fragColorOut = vec4(color, 1.0);
    gl_FragDepth = log2(max(1e-6, flogz)) / log2(u_far + 1.0);
}
)";

// --- Scale constants ---

namespace
{
    constexpr float kEarthRadius = 1.0f;
} // namespace

// --- GLViewport ---

GLViewport::GLViewport(QWidget *parent)
    : QOpenGLWidget(parent)
{
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setDepthBufferSize(24);
    setFormat(fmt);
}

GLViewport::~GLViewport()
{
    makeCurrent();
    m_earthVAO.destroy();
    m_earthVBO.destroy();
    m_earthEBO.destroy();
    m_axesVAO.destroy();
    m_axesVBO.destroy();
    m_sunVAO.destroy();
    m_sunVBO.destroy();
    m_sunEBO.destroy();
    m_satVAO.destroy();
    m_satVBO.destroy();
    m_satEBO.destroy();
    m_markerVAO.destroy();
    m_markerVBO.destroy();
    if (m_earthTexture)
        glDeleteTextures(1, &m_earthTexture);
    delete m_litShader;
    delete m_unlitShader;
    doneCurrent();
}

// initializeGL() is a virtual method that QOpenGLWidget calls automatically.
// Qt invokes it once when:
//  The OpenGL context is first created
//  The widget is first shown/rendered
void GLViewport::initializeGL()
{
    if (!initializeOpenGLFunctions())
    {
        qCritical() << "Failed to initialize OpenGL 3.3 Core functions";
        return;
    }

    qInfo() << "OpenGL version:" << reinterpret_cast<const char *>(glGetString(GL_VERSION));

    glClearColor(0.02f, 0.02f, 0.05f, 1.0f);

    // glEnable depth testing for correct occlusion of 3D objects is a must-have
    // for any 3D scene.
    // If you forget this, you'll see weird visual artifacts where objects don't
    // properly overlap each other.
    glEnable(GL_DEPTH_TEST);

    initShaders();
    setupEarth();
    setupAxes();
    setupSunIndicator();
    setupSatellite();
    setupOrientationMarker();
}

void GLViewport::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void GLViewport::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspect = float(width()) / float(height() ? height() : 1);
    QMatrix4x4 view = m_camera.viewMatrix();
    QMatrix4x4 projection = m_camera.projectionMatrix(aspect);

    // --- Draw Earth ---
    if (m_litShader && m_earthIndexCount > 0)
    {
        m_litShader->bind();

        QMatrix4x4 model;
        m_litShader->setUniformValue("model", model);
        m_litShader->setUniformValue("view", view);
        m_litShader->setUniformValue("projection", projection);
        m_litShader->setUniformValue("normalMatrix", model.normalMatrix());
        m_litShader->setUniformValue("u_far", m_camera.farClip());

        // Sun light direction (matches VTK scene)
        m_litShader->setUniformValue("lightDir", QVector3D(1.0f, 0.3f, 0.2f).normalized());
        m_litShader->setUniformValue("lightColor", QVector3D(1.0f, 0.98f, 0.95f));
        m_litShader->setUniformValue("ambientColor", QVector3D(0.048f, 0.048f, 0.064f));
        m_litShader->setUniformValue("materialColor", QVector3D(0.2f, 0.4f, 0.7f));
        m_litShader->setUniformValue("hasTexture", m_earthHasTexture);

        if (m_earthHasTexture)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_earthTexture);
            m_litShader->setUniformValue("textureSampler", 0);
        }

        m_earthVAO.bind();
        glDrawElements(GL_TRIANGLES, m_earthIndexCount, GL_UNSIGNED_INT, nullptr);
        m_earthVAO.release();

        // --- Draw Satellite ---
        if (m_satIndexCount > 0)
        {
            m_litShader->setUniformValue("model", m_satModel);
            m_litShader->setUniformValue("normalMatrix", m_satModel.normalMatrix());
            m_litShader->setUniformValue("hasTexture", false);
            m_litShader->setUniformValue("materialColor", QVector3D(0.7f, 0.7f, 0.7f));

            m_satVAO.bind();
            glDrawElements(GL_TRIANGLES, m_satIndexCount, GL_UNSIGNED_INT, nullptr);
            m_satVAO.release();
        }

        m_litShader->release();
    }

    // --- Draw ECI axes ---
    if (m_unlitShader)
    {
        m_unlitShader->bind();
        QMatrix4x4 mvp = projection * view;
        m_unlitShader->setUniformValue("mvp", mvp);
        m_unlitShader->setUniformValue("u_far", m_camera.farClip());

        m_axesVAO.bind();
        glLineWidth(2.0f);
        m_unlitShader->setUniformValue("color", QVector3D(1.0f, 0.2f, 0.2f));
        glDrawArrays(GL_LINES, 0, 2);
        m_unlitShader->setUniformValue("color", QVector3D(0.2f, 1.0f, 0.2f));
        glDrawArrays(GL_LINES, 2, 2);
        m_unlitShader->setUniformValue("color", QVector3D(0.4f, 0.4f, 1.0f));
        glDrawArrays(GL_LINES, 4, 2);
        glLineWidth(1.0f);
        m_axesVAO.release();

        // --- Draw Sun indicator ---
        QMatrix4x4 sunModel;
        sunModel.translate(12.0f, 3.6f, 2.4f);
        QMatrix4x4 sunMVP = projection * view * sunModel;
        m_unlitShader->setUniformValue("mvp", sunMVP);
        m_unlitShader->setUniformValue("color", QVector3D(1.0f, 0.95f, 0.4f));

        m_sunVAO.bind();
        glDrawElements(GL_TRIANGLES, m_sunIndexCount, GL_UNSIGNED_INT, nullptr);
        m_sunVAO.release();

        m_unlitShader->release();
    }

    // --- Orientation marker (corner axes) ---
    drawOrientationMarker(view);
}

void GLViewport::initShaders()
{
    // Create the "lit" shader program for objects with lighting (e.g., Earth, satellite)
    m_litShader = new QOpenGLShaderProgram(this);
    // Compile vertex shader (runs for each vertex; typically handles transformations and lighting calculations)
    if (!m_litShader->addShaderFromSourceCode(QOpenGLShader::Vertex, kLitVertexShader))
        qCritical() << "Lit vertex shader failed:" << m_litShader->log();
    // Compile fragment shader (runs for each pixel; computes final color with lighting)
    if (!m_litShader->addShaderFromSourceCode(QOpenGLShader::Fragment, kLitFragmentShader))
        qCritical() << "Lit fragment shader failed:" << m_litShader->log();
    // Link the program (combines vertex + fragment shaders into a single executable program)
    if (!m_litShader->link())
        qCritical() << "Lit shader link failed:" << m_litShader->log();

    // Create the "unlit" shader program for objects without lighting (e.g., coordinate axes, sun indicator)
    m_unlitShader = new QOpenGLShaderProgram(this);
    // Compile vertex shader
    if (!m_unlitShader->addShaderFromSourceCode(QOpenGLShader::Vertex, kUnlitVertexShader))
        qCritical() << "Unlit vertex shader failed:" << m_unlitShader->log();
    // Compile fragment shader (typically just outputs a fixed color without lighting calculations)
    if (!m_unlitShader->addShaderFromSourceCode(QOpenGLShader::Fragment, kUnlitFragmentShader))
        qCritical() << "Unlit fragment shader failed:" << m_unlitShader->log();
    // Link the program
    if (!m_unlitShader->link())
        qCritical() << "Unlit shader link failed:" << m_unlitShader->log();
}

void GLViewport::setupEarth()
{
    MeshData sphere = generateUVSphere(kEarthRadius, 64, 64);
    m_earthIndexCount = static_cast<int>(sphere.indices.size());

    m_earthVAO.create();
    m_earthVAO.bind();

    m_earthVBO.create();
    m_earthVBO.bind();
    m_earthVBO.allocate(sphere.vertices.data(),
                        static_cast<int>(sphere.vertices.size() * sizeof(float)));

    // position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, MeshData::stride,
                          reinterpret_cast<void *>(MeshData::positionOffset));
    // normal (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, MeshData::stride,
                          reinterpret_cast<void *>(MeshData::normalOffset));
    // texcoord (location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, MeshData::stride,
                          reinterpret_cast<void *>(MeshData::texCoordOffset));

    m_earthEBO.create();
    m_earthEBO.bind();
    m_earthEBO.allocate(sphere.indices.data(),
                        static_cast<int>(sphere.indices.size() * sizeof(uint32_t)));

    m_earthVAO.release();

    // Load Earth texture
    QString texPath = findFile("textures/earth_bluemarble_2k.jpg");
    if (!texPath.isEmpty())
    {
        QImage img(texPath);
        if (!img.isNull())
        {
            img = img.convertToFormat(QImage::Format_RGBA8888).mirrored();

            glGenTextures(1, &m_earthTexture);
            glBindTexture(GL_TEXTURE_2D, m_earthTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, img.constBits());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);

            m_earthHasTexture = true;
            qInfo() << "Earth texture loaded:" << texPath;
        }
    }

    if (!m_earthHasTexture)
        qInfo() << "Earth texture not found; using solid blue fallback.";
}

void GLViewport::setupSatellite()
{
    constexpr float kOrbitalRadius = 1.0628f;
    constexpr float kSatelliteTargetSize = 0.04f;

    QString modelPath = findFile("models/satellite.obj");

    MeshData mesh;
    BoundingBox bounds{};

    if (!modelPath.isEmpty())
    {
        ObjResult obj = loadObj(modelPath.toStdString());
        if (!obj.mesh.indices.empty())
        {
            mesh = std::move(obj.mesh);
            bounds = obj.bounds;
            qInfo() << "Satellite OBJ loaded:" << modelPath
                    << "vertices:" << mesh.vertices.size() / 8
                    << "triangles:" << mesh.indices.size() / 3;
        }
    }

    // Fallback to small sphere if OBJ missing
    if (mesh.indices.empty())
    {
        qInfo() << "No OBJ found; using fallback sphere.";
        mesh = generateUVSphere(0.02f, 16, 16);
        bounds = {{-0.02f, -0.02f, -0.02f}, {0.02f, 0.02f, 0.02f}};
    }

    m_satIndexCount = static_cast<int>(mesh.indices.size());

    float extent = bounds.maxExtent();
    m_satScale = (extent > 0.0f) ? kSatelliteTargetSize / extent : 1.0f;
    bounds.center(m_satCenter);

    m_satVAO.create();
    m_satVAO.bind();

    m_satVBO.create();
    m_satVBO.bind();
    m_satVBO.allocate(mesh.vertices.data(),
                      static_cast<int>(mesh.vertices.size() * sizeof(float)));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, MeshData::stride,
                          reinterpret_cast<void *>(MeshData::positionOffset));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, MeshData::stride,
                          reinterpret_cast<void *>(MeshData::normalOffset));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, MeshData::stride,
                          reinterpret_cast<void *>(MeshData::texCoordOffset));

    m_satEBO.create();
    m_satEBO.bind();
    m_satEBO.allocate(mesh.indices.data(),
                      static_cast<int>(mesh.indices.size() * sizeof(uint32_t)));

    m_satVAO.release();

    rebuildSatelliteModel();
}

void GLViewport::setSatelliteOrientation(float roll, float pitch, float yaw)
{
    m_satRoll = roll;
    m_satPitch = pitch;
    m_satYaw = yaw;
    rebuildSatelliteModel();
    update();
}

void GLViewport::rebuildSatelliteModel()
{
    constexpr float kOrbitalRadius = 1.0628f;

    m_satModel.setToIdentity();
    m_satModel.translate(kOrbitalRadius, 0.0f, 0.0f);
    // VTK rotation order: X then Y then Z => matrix: Rz * Ry * Rx
    m_satModel.rotate(m_satYaw, 0, 0, 1);
    m_satModel.rotate(m_satPitch, 0, 1, 0);
    m_satModel.rotate(m_satRoll, 1, 0, 0);
    m_satModel.scale(m_satScale);
    m_satModel.translate(-m_satCenter[0], -m_satCenter[1], -m_satCenter[2]);
}

void GLViewport::setupOrientationMarker()
{
    // Unit-length axes for the corner marker
    // clang-format off
    float markerVertices[] = {
        0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
    };
    // clang-format on

    m_markerVAO.create();
    m_markerVAO.bind();

    m_markerVBO.create();
    m_markerVBO.bind();
    m_markerVBO.allocate(markerVertices, sizeof(markerVertices));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    m_markerVAO.release();
}

void GLViewport::drawOrientationMarker(const QMatrix4x4 &view)
{
    if (!m_unlitShader)
        return;

    int cornerW = static_cast<int>(width() * 0.2);
    int cornerH = static_cast<int>(height() * 0.2);

    glViewport(0, 0, cornerW, cornerH);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Use only the rotation part of the view matrix (no translation)
    QMatrix4x4 rotOnly;
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            rotOnly(r, c) = view(r, c);

    QMatrix4x4 ortho;
    ortho.ortho(-1.5f, 1.5f, -1.5f, 1.5f, -10.0f, 10.0f);

    m_unlitShader->bind();
    m_markerVAO.bind();

    glLineWidth(2.0f);
    QMatrix4x4 mvp = ortho * rotOnly;
    m_unlitShader->setUniformValue("mvp", mvp);
    m_unlitShader->setUniformValue("u_far", m_camera.farClip());

    m_unlitShader->setUniformValue("color", QVector3D(1.0f, 0.2f, 0.2f));
    glDrawArrays(GL_LINES, 0, 2);
    m_unlitShader->setUniformValue("color", QVector3D(0.2f, 1.0f, 0.2f));
    glDrawArrays(GL_LINES, 2, 2);
    m_unlitShader->setUniformValue("color", QVector3D(0.4f, 0.4f, 1.0f));
    glDrawArrays(GL_LINES, 4, 2);
    glLineWidth(1.0f);

    m_markerVAO.release();
    m_unlitShader->release();

    // Restore full viewport
    glViewport(0, 0, width(), height());
}

void GLViewport::setupAxes()
{
    constexpr float kEciAxisLength = 1.5f;

    // clang-format off
    float axisVertices[] = {
        // X axis (red)
        0.0f, 0.0f, 0.0f,
        kEciAxisLength, 0.0f, 0.0f,
        // Y axis (green)
        0.0f, 0.0f, 0.0f,
        0.0f, kEciAxisLength, 0.0f,
        // Z axis (blue)
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, kEciAxisLength,
    };
    // clang-format on

    m_axesVAO.create();
    m_axesVAO.bind();

    m_axesVBO.create();
    m_axesVBO.bind();
    m_axesVBO.allocate(axisVertices, sizeof(axisVertices));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    m_axesVAO.release();
}

void GLViewport::setupSunIndicator()
{
    MeshData sphere = generateUVSphere(0.3f, 16, 16);
    m_sunIndexCount = static_cast<int>(sphere.indices.size());

    m_sunVAO.create();
    m_sunVAO.bind();

    m_sunVBO.create();
    m_sunVBO.bind();
    m_sunVBO.allocate(sphere.vertices.data(),
                      static_cast<int>(sphere.vertices.size() * sizeof(float)));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, MeshData::stride,
                          reinterpret_cast<void *>(MeshData::positionOffset));

    m_sunEBO.create();
    m_sunEBO.bind();
    m_sunEBO.allocate(sphere.indices.data(),
                      static_cast<int>(sphere.indices.size() * sizeof(uint32_t)));

    m_sunVAO.release();
}

void GLViewport::mousePressEvent(QMouseEvent *event)
{
    m_activeButton = event->button();
    m_lastMousePos = event->pos();
}

void GLViewport::mouseMoveEvent(QMouseEvent *event)
{
    QPoint delta = event->pos() - m_lastMousePos;
    m_lastMousePos = event->pos();

    if (m_activeButton == Qt::LeftButton)
    {
        m_camera.rotate(-delta.x() * 0.3f, -delta.y() * 0.3f);
        update();
    }
    else if (m_activeButton == Qt::MiddleButton)
    {
        m_camera.pan(delta.x(), delta.y(), width(), height());
        update();
    }
    else if (m_activeButton == Qt::RightButton)
    {
        m_camera.zoom(delta.y() * 0.3f);
        update();
    }
}

void GLViewport::mouseReleaseEvent(QMouseEvent * /*event*/)
{
    m_activeButton = Qt::NoButton;
}

void GLViewport::wheelEvent(QWheelEvent *event)
{
    float delta = event->angleDelta().y() / 120.0f;
    m_camera.zoom(delta);
    update();
}

QString GLViewport::findFile(const QString &relativePath) const
{
    QStringList searchPaths = {
        QCoreApplication::applicationDirPath() + "/" + relativePath,
        relativePath,
    };

    for (const QString &p : searchPaths)
    {
        if (QFile::exists(p))
            return p;
    }

    qWarning() << "File not found:" << relativePath;
    return {};
}
