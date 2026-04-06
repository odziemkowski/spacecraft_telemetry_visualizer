#pragma once

#include <QString>

class QWidget;
class GLViewport;

class SceneManager
{
public:
    explicit SceneManager(QWidget *parent = nullptr);

    void setup();
    void updateOrientation(int roll, int pitch, int yaw);
    QWidget *widget();

private:
    GLViewport *m_viewport;
};
