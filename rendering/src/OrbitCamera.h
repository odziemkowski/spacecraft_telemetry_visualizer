#pragma once

#include <QMatrix4x4>
#include <QVector3D>

class OrbitCamera
{
public:
    OrbitCamera();

    void rotate(float dAzimuth, float dElevation);
    void zoom(float delta);
    void pan(float dx, float dy, float viewportW, float viewportH);

    QMatrix4x4 viewMatrix() const;
    QMatrix4x4 projectionMatrix(float aspect) const;
    QVector3D eyePosition() const;

    void setDistance(float d);
    float distance() const { return m_distance; }
    float farClip() const { return m_farClip; }

private:
    float m_distance = 3.0f;
    float m_azimuth = 30.0f;   // degrees
    float m_elevation = 20.0f; // degrees
    QVector3D m_focalPoint{0.0f, 0.0f, 0.0f};

    float m_fov = 45.0f;
    float m_nearClip = 0.001f;
    float m_farClip = 100.0f;
};
