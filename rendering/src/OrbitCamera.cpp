#include "OrbitCamera.h"

#include <QtMath>
#include <algorithm>

OrbitCamera::OrbitCamera() = default;

void OrbitCamera::rotate(float dAzimuth, float dElevation)
{
    m_azimuth += dAzimuth;
    m_elevation = std::clamp(m_elevation + dElevation, -89.0f, 89.0f);
}

void OrbitCamera::zoom(float delta)
{
    m_distance *= (1.0f - delta * 0.1f);
    m_distance = std::clamp(m_distance, 0.1f, 50.0f);
}

void OrbitCamera::pan(float dx, float dy, float viewportW, float viewportH)
{
    // Pan speed proportional to distance and viewport
    float scale = m_distance * 2.0f * std::tan(qDegreesToRadians(m_fov * 0.5f));

    QMatrix4x4 view = viewMatrix();
    QVector3D right(view(0, 0), view(0, 1), view(0, 2));
    QVector3D up(view(1, 0), view(1, 1), view(1, 2));

    float panX = -dx / viewportW * scale;
    float panY = dy / viewportH * scale;

    m_focalPoint += right * panX + up * panY;
}

QMatrix4x4 OrbitCamera::viewMatrix() const
{
    QVector3D eye = eyePosition();

    QMatrix4x4 view;
    view.lookAt(eye, m_focalPoint, QVector3D(0, 1, 0));
    return view;
}

QMatrix4x4 OrbitCamera::projectionMatrix(float aspect) const
{
    QMatrix4x4 proj;
    proj.perspective(m_fov, aspect, m_nearClip, m_farClip);
    return proj;
}

QVector3D OrbitCamera::eyePosition() const
{
    float azRad = qDegreesToRadians(m_azimuth);
    float elRad = qDegreesToRadians(m_elevation);

    float x = m_distance * std::cos(elRad) * std::cos(azRad);
    float y = m_distance * std::sin(elRad);
    float z = m_distance * std::cos(elRad) * std::sin(azRad);

    return m_focalPoint + QVector3D(x, y, z);
}

void OrbitCamera::setDistance(float d)
{
    m_distance = std::clamp(d, 0.1f, 50.0f);
}
