#pragma once

#include "OrbitCamera.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QPoint>
#include <QString>

class GLViewport : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    explicit GLViewport(QWidget *parent = nullptr);
    ~GLViewport() override;

    void setSatelliteOrientation(float roll, float pitch, float yaw);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void initShaders();
    void setupEarth();
    void setupAxes();
    void setupSunIndicator();
    void setupSatellite();
    void rebuildSatelliteModel();
    void setupOrientationMarker();
    void drawOrientationMarker(const QMatrix4x4 &view);
    QString findFile(const QString &relativePath) const;

    QOpenGLShaderProgram *m_litShader = nullptr;
    QOpenGLShaderProgram *m_unlitShader = nullptr;

    // Earth
    QOpenGLVertexArrayObject m_earthVAO;
    QOpenGLBuffer m_earthVBO{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_earthEBO{QOpenGLBuffer::IndexBuffer};
    int m_earthIndexCount = 0;
    GLuint m_earthTexture = 0;
    bool m_earthHasTexture = false;

    // ECI axes
    QOpenGLVertexArrayObject m_axesVAO;
    QOpenGLBuffer m_axesVBO{QOpenGLBuffer::VertexBuffer};

    // Sun indicator
    QOpenGLVertexArrayObject m_sunVAO;
    QOpenGLBuffer m_sunVBO{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_sunEBO{QOpenGLBuffer::IndexBuffer};
    int m_sunIndexCount = 0;

    // Orientation marker
    QOpenGLVertexArrayObject m_markerVAO;
    QOpenGLBuffer m_markerVBO{QOpenGLBuffer::VertexBuffer};

    // Satellite
    QOpenGLVertexArrayObject m_satVAO;
    QOpenGLBuffer m_satVBO{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_satEBO{QOpenGLBuffer::IndexBuffer};
    int m_satIndexCount = 0;
    QMatrix4x4 m_satModel;
    float m_satScale = 1.0f;
    float m_satCenter[3] = {0, 0, 0};
    float m_satRoll = 0, m_satPitch = 0, m_satYaw = 0;

    // Camera
    OrbitCamera m_camera;
    QPoint m_lastMousePos;
    Qt::MouseButton m_activeButton = Qt::NoButton;
};
