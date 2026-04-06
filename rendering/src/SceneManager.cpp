#include "SceneManager.h"
#include "GLViewport.h"

SceneManager::SceneManager(QWidget *parent)
    : m_viewport(new GLViewport(parent))
{
}

void SceneManager::setup()
{
    // GL resources are created in GLViewport::initializeGL(),
    // which runs automatically when the widget is first shown.
}

void SceneManager::updateOrientation(int roll, int pitch, int yaw)
{
    m_viewport->setSatelliteOrientation(
        static_cast<float>(roll),
        static_cast<float>(pitch),
        static_cast<float>(yaw));
}

QWidget *SceneManager::widget()
{
    return m_viewport;
}
